#include <iostream>
#include "two_stage.h"

int main() {

    TwoStageSCS sp("tests", "lands", 12);
    sp.solve();
    return 0;
}

