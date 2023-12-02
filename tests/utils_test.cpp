#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "utils.h"

TEST_CASE("BijectiveMap operations", "[BijectiveMap]") {
    BijectiveMap map;

    SECTION("Adding and retrieving name-index pairs") {
        map.add("Alice", 0);
        map.add("Bob", 1);
        map.add("Charlie", 2);

        REQUIRE(map.get_name(0).value() == "Alice");
        REQUIRE(map.get_name(1).value() == "Bob");
        REQUIRE(map.get_name(2).value() == "Charlie");

        REQUIRE(map.get_index("Alice").value() == 0);
        REQUIRE(map.get_index("Bob").value() == 1);
        REQUIRE(map.get_index("Charlie").value() == 2);
    }

    SECTION("Handling non-existent names and indices") {
        map.add("Alice", 0);
        map.add("Bob", 1);

        // Check for std::nullopt instead of expecting exceptions
        REQUIRE_FALSE(map.get_name(3).has_value());
        REQUIRE_FALSE(map.get_index("NonExistent").has_value());
    }
}
