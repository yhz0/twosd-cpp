#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "utils.h"

TEST_CASE("BijectiveMap operations", "[BijectiveMap]") {
    BijectiveMap map;

    SECTION("Adding and retrieving name-index pairs") {
        map.add("Alice", 0);
        map.add("Bob", 1);
        map.add("Charlie", 2);

        REQUIRE(map.get_name(0) == "Alice");
        REQUIRE(map.get_name(1) == "Bob");
        REQUIRE(map.get_name(2) == "Charlie");

        REQUIRE(map.get_index("Alice") == 0);
        REQUIRE(map.get_index("Bob") == 1);
        REQUIRE(map.get_index("Charlie") == 2);
    }

    SECTION("Handling non-existent names and indices") {
        map.add("Alice", 0);
        map.add("Bob", 1);

        REQUIRE_THROWS_AS(map.get_name(3), std::runtime_error);
        REQUIRE_THROWS_AS(map.get_index("NonExistent"), std::runtime_error);
    }
}
