# Parallel Implementation of Stochastic Decomposition Algorithm


## An application: network capacity planning

- Given a graph (V, E). Nodes represent cities or regions. Edges represent network links.
- There are communication demand between some pairs of link, and these demand can be satisfied by multiple hops through the links, say at most 3 hops.
- The demand is not known ahead of time, but we only have a rough estimate of the demand. We model this as uncertainty.
- After the demand is revealed, the communications are routed on the network. If the demand could not be satisfied, they are lost.
- Each unit of capacity on the network cost a certain amount of money.
- We are given a budget $B$.
- Objective: how to invest in the links to minimize the expected number of lost demand?


## Problem: Two-stage Stochastic LP with fixed recourse

$$
\begin{align*}
\min_x \quad & c^\top x + E[h(x, \tilde \omega)]\\
s.t.\quad & Ax = b, x \geq 0
\end{align*}
$$

$$
h(x,\xi) = \min \{d^\top y: T(\omega)x + Wy = r(\omega), \quad y\geq 0\}
$$

$x$ is called a __first-stage decision__ (e.g., link capacity decision). $\omega$ represents the uncertainty (demand between each pair). $y$ is called the __recourse decision__ (e.g., given the link capacity and demand, how to optimally route the communications). We usually write $Q(x) = E[h(x, \tilde \omega)]$, and call it __expected value function__.

The challenge is two-fold:
1. $h$ is convex in $x$ but non-differentiable.
2. Evaluation of a single scenario $h(\cdot, \omega)$ is expensive. Exact evaluation of the expected value function $Q(x)$ is usually not possible. Only statistical approximation is available.

Standard assumptions:
1. Relatively complete recourse: if $x$ is first stage feasible, then there exists a feasible $y$ in the second stage almost surely.
2. The problem is bounded from below.
3. The first stage feasible set is non-empty (otherwise no need to solve) and compact (can be relaxed).


Some algorithms:
- Stochastic Quasi-gradient (similar to SGD)
- Deterministic Equivalent Problem
- Benders Decomposition (temporal decomposition)
- Progressive Hedging (scenario decomposition + ADMM; mpi-sppy in python)

The well-known Stochastic Decomposition algorithm of Higle and Sen (Math. of OR 16(3), 650-669, 1991) is one of the well-known algorithms to solve this problem. In this work we 

## Deterministic Equivalent

If the random element $\omega$ has finite support i.e. there are only a finite number of possible realizations of $T, r$ pairs, we can represent the problem into a large scale linear program.

If $T(\omega), r(\omega)$ takes value $T_i, r_i$ with probability $p_i$, $i = 1 \dots N$.

$\min_{x, y_i} c^\top x + \sum_i p_i d^\top y_i$

subject to $Ax = b, x \geq 0$

$T_i x + Wy_i = r_i, y_i \geq 0 \ \forall i = 1 \dots N$

Additional assumption: Fixed recourse -- $W$ does not depend on $\omega$.

Drawback: This LP has $n_x + N n_y$ variables. If $N$ is large, the linear program blows up quickly.

## Sketch of Stochastic Decomposition Algorithm

1. Sample Average Approximation (SAA): replace the expectation with an $\omega_1, \dots, \omega_N$. The number of samples increase gradually.
2. Cutting plane algorithm: given $x$, we approximately(*) solve all the scenario problems and use the convexity of $h$ to update piecewise linear lower bound of the second stage expected value function $\hat Q(x)$.
3. A regularized master problem is solved to obtain a new $x$. The cutting planes should be updated at the new $x$. Unnecessary cuts (inequalities) should be removed to keep the size of master problem in check.

## Design One: Maintain Independent Approximations

In the original stochastic decomposition algorithm, the master problem keeps track of an approximation of the expected value function $\hat Q(x) \approx Q(x)$. Rather than relying on a single approximation, this work utilizes a set of approximations $Q_​j​(x)$, each representing an independent approximation of the same function. The master problem will use the weighted average of these approximations $\sum \alpha_j \hat Q_j(x)$.

So the problem will look like
$$\min_{x \in X} c^\top x + \sum \alpha_j \hat Q_j(x)$$

There are two primary advantages for doing this:

1. Increased Sample Size: Having a large number of samples through $\hat Q_j​(x)$ significantly reduces variance.
2. Parallelization: The independent nature of constructing each $Q_j(x)$ allows for the distribution of computational tasks across multiple workers. This parallelization not only speeds up the process but also makes efficient use of available computational resources.

### Implementation Plan (MPI + OpenMP)

We construct two types of ranks: the Master and the Workers.

In this work I will use one-sided MPI Communications. Each Worker is responsible for setting up a dedicated memory space, known as a 'window', which is accessible to other ranks.

The synchronization is achieved passively. Each rank initially processes its assigned tasks using a local copy of the variables. Upon completion, a rank secures an exclusive lock (MPI_Win_lock) on its designated window, and copy the local results to the window. This lock mechanism prevents other ranks from getting corrupt data.

The Master should maintain the master problem. It shares an iteration number that works as a timestamp for all ranks, the current incumbent solution (stable solution) and the candidate solution.

1. Check the STOP flag. If STOP flag is present, notify all workers to terminate, print results and quit.
2. Obtain approximations $\hat Q_j$ from Workers. Calculate weights $\alpha_j$ from the number of samples in each worker.
3. Form the (regularized) master problem and solve it using a QP solver.
4. Obtain locks to windows and update timestamp and candidate solutions.

The workers need a little bit more coordination. It keeps track of the timestamp, and each time at the beginning it should fetch the timestamp from the master. If the fetched timestamp is the same the local copy, it means the incumbent solution have not changed. In order to keep the "lag" between master and workers bounded, it can only sample a number of times before it is forced to wait.

1. Set current_sample_count = 0. Get timestamp from Master.
2. If current_sample_count < MAX_SAMPLE_COUNT or timestamp has changed, perform approximation step. Increment current_sample_count.
3. Otherwise, wait and try again.

The approximation step has the same steps as the original stochastic decomposition algorithm, which are removing slack cuts, solving new scenario LPs and argmax procedure to approximate old scenario LPs.

## Design Two: Offloading Batched Linear Algebra Operations in Argmax procedure (CUDA)

In the original stochastic decomposition algorithm, the argmax procedure is the key part of this algorithm, and takes more than half of the cpu time. The argmax procedure can be described as follows:

Given $x$, dual extreme points $\pi_i$ and sample set $\omega_j$, find 

$$\max_j \pi_i(r(\omega_j) - T(\omega_j)x)$$

If the given dual extreme points set is reasonably large (~2000), then this approximation will be accurate. In addition, if the number of samples is large, then we have smaller sample variance.

We note that this operation requires a double loop in $i$ and $j$, and a max reduction step, therefore it may be worthwhile to offload the computation on GPU. However, the implementation of this is not straightforward.

### Separating Fixed Part and Random Part

Although $\pi_i$ are usually dense, $r(\omega_j), T(\omega_j)$ are sparse, and GPUs are not good at sparse operations. Here we exploit the fact that in almost all problems, $r$ and $T$ can be decomposed into a fixed part and a random part. The fixed part is shared across all $\omega$'s and the random part is extremely sparse. We may write,

$$r(\omega_j) = \bar r + \Delta r(\omega_j) \\ T(\omega_j) = \bar T + \Delta T(\omega_j)$$

Therefore, 
$$\pi_i(r(\omega_j) - T(\omega_j)x) = \pi_i \bar r - \pi \bar T x + \pi_i \Delta r(\omega_j) - \pi_i \Delta T(\omega_j)x = \bar \alpha_i +\bar \beta_ix + \alpha_{ij}$$

There are four coefficients here, we denote $\bar \alpha_i = \pi_i \bar r$, $\beta_i = - \pi_i \bar T$, $\alpha_{ij} = \pi_i \Delta r(\omega_j)$, $\beta_{ij} = - \pi_i \Delta T(\omega_j)$.

The first two coefficients, corresponding to the fixed part, are usually dense. To obtain them, a sparse matrix computation is needed so we compute them on CPU. This computation can be carried out as soon as the new dual extreme point is discovered. We will copy the fixed coefficients to GPU for further use.

### Preparing $\alpha_{ij}$ and $\beta_{ij}$

The next two coefficients are the one we are going to offload to the GPU. Before copying, we arrange the non-zero elements in $\Delta r(\omega)$ and $\Delta T(\omega)$ into a (dense) vector, and correspondingly create a re-indexed and truncated copy of all $\pi$ and $x$, such that the calculation of $\alpha_{ij}$ and $\beta_{ij}$ can be reduced to a dense dot product and element-wise multiplication, which can be efficiently done on GPU.

We will proceed to copy all transformed version of $\pi$, $\Delta r(\omega)$ and $\Delta T(\omega)$ in device global memory, and use a kernel to compute these coefficients, with the results stored into global memory.

We note that these coefficients do not depend on $x$ in any way, so they can be re-used. As the algorithm proceeds, more $\pi$ and $\omega$ will be added, and only the necessary computations will be carried out.

### Batched Linear Operations

![GPU_1](https://github.com/yhz0/twosd-cpp/assets/35647076/48979b64-866e-4f77-b909-327344dc46e8)


With all the coefficients ready, $x$ and the shortened $x$ will be copied into the read-only device constant memory. The device will do the following:

1. For the fixed part, an SGEMV matrix-vector multiplication to compute $\bar \alpha_i + \bar \beta_i x$.
2. For the random part, a batched SGEMV matrix-vector multiplication to compute $\alpha_{ij} + \beta_{ij} x$.
3. Add the fixed part result to the random part result to recover $\pi_i(r(\omega_j) - T(\omega_j)x)$.

### Parallel Reduction

We may allocate shared memory to store the result of the previous calculation, since they are now all scalars. The argmax operation will be done in this array on shared memory in two passes.
![GPU_2](https://github.com/yhz0/twosd-cpp/assets/35647076/35551894-5265-4304-9030-e9e3b2b517b2)


1. The first pass will use a parallel reduction scheme to find the max value of the array across the $\pi$ axis. If more than one block is used in the $\pi$ axis, another round of reduction is needed.
2. After the first pass completes, the second pass will conditionally copy the $\bar \alpha_i + \alpha_{ij}$ and $\bar \beta_i+\beta_{ij}$ to another array.
3. Optionally, these values will be averaged, and the resulting coefficients copied to the host.

## Design Three: validate the solutions online.

One shortcoming in the original stochastic decomposition algorithm is that it is difficult to get a stopping criteria, since we have to rely on statistical estimations. It is desirable that we have a way to monitor the progress the algorithm is making, by calculating an upper bound of the true objective. We can create another rank called Validator, that fetches the incumbent solution $x$ from the master. It will attempt to evaluate the true $Q(x)$ at that point, by external sampling and solving second stage scenario LPs. Once the pessimistic gap is small enough, we can declare the problem is solved to near-optimality.
