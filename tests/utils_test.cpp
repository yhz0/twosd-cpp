#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "utils.h"

TEST_CASE("BijectiveMap operations", "[BijectiveMap]") {
    BijectiveMap map;

    SECTION("Adding and retrieving name-index pairs") {
        map.add("Alice", 0);
        map.add("Bob", 1);
        map.add("Charlie", 2);

        REQUIRE(map.getName(0) == "Alice");
        REQUIRE(map.getName(1) == "Bob");
        REQUIRE(map.getName(2) == "Charlie");

        REQUIRE(map.getIndex("Alice") == 0);
        REQUIRE(map.getIndex("Bob") == 1);
        REQUIRE(map.getIndex("Charlie") == 2);
    }

    SECTION("Handling non-existent names and indices") {
        map.add("Alice", 0);
        map.add("Bob", 1);

        REQUIRE_THROWS_AS(map.getName(3), std::runtime_error);
        REQUIRE_THROWS_AS(map.getIndex("NonExistent"), std::runtime_error);
    }
}
