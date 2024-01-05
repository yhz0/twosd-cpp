#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include "../external/catch_amalgamated.hpp"
#include "vector_container.h"

using Catch::Approx;

TEST_CASE("VectorContainer Construction and Basic Operations", "[VectorContainer]") {
    VectorContainer vc(10, 3);  // Container for 10 vectors, each of dimension 3

    SECTION("Initial State") {
        REQUIRE(vc.size() == 0);
        REQUIRE(vc.get_vector_dims() == 3);
        // REQUIRE(vc.get_padding_dims() == 5);  // Padding to the nearest multiple of 8
    }

    SECTION("Insertion and Retrieval") {
        std::optional<size_t> pos;

        // insert 5 elements
        // (0 1 2)  (100 101 102) ...
        for (int i = 0; i < 5; ++i)
        {
            std::vector<float> vec = {
                (float)(100.0 * i),
                (float)(100.0 * i + 1.0),
                (float)(100.0 * i + 2.0)};
            pos = vc.insert(vec);
        }

        std::vector<float> expected = {400.0f, 401.0f, 402.0f};
        CHECK(pos.value() == 4);
        CHECK(vc.size() == 5);
        CHECK(vc.get(4) == expected);

        // insert 10 more elements
        for (int i = 5; i < 15; ++i)
        {
            std::vector<float> vec = {
                (float)(100.0 * i),
                (float)(100.0 * i + 1.0),
                (float)(100.0 * i + 2.0)};
            pos = vc.insert(vec);
        }

        CHECK(pos.value() == 4);
        CHECK(vc.size() == 10);
        
        // lets get the first element now. should be 
        // (1000 1001 1002)
        std::vector<float> new_expected = {1000.0f, 1001.0f, 1002.0f};
        CHECK(vc.get(0) == new_expected);
    }
}

TEST_CASE("VectorContainer Synchronization Tracking", "[VectorContainer]") {

    SECTION("Tracking Updated Positions") {
        VectorContainer vc(5, 3); // Smaller container of size 5
        // insert (0 0 0) (1 1 1) (2 2 2)
        for (int i = 0; i < 3; ++i)
        {
            vc.insert(std::vector<float>{static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
        }
        CHECK(vc.get_current_position() == 3);
        vc.reset_sync_range();

        // insert (3 3 3)
        vc.insert(std::vector<float>{3.0f, 3.0f, 3.0f});
        CHECK(vc.get_wrap_around_flag() == false);
        CHECK(vc.get_sync_position() == 3);
        CHECK(vc.get_current_position() == 4);
    }

    SECTION("Wrap-Around Handling") {
        VectorContainer vc(5, 3); // container of size 5
        // (0 0 0) (1 1 1) (2 2 2) (3 3 3)
        for (int i = 0; i < 4; ++i) {  // Insert more than the capacity to trigger wrap-around
            vc.insert(std::vector<float>{static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
        }
        vc.reset_sync_range();

        vc.insert(std::vector<float>{4.0f, 4.0f, 4.0f});
        CHECK(vc.get_wrap_around_flag() == true);
        CHECK(vc.get_sync_position() == 4);
        CHECK(vc.get_current_position() == 0);
    }

    SECTION("Full track edge case") {
        VectorContainer vc(5, 3); // container of size 5
        // insert 12 elements
        for (int i = 0; i < 12; ++i)
        {
            vc.insert(std::vector<float>{static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
        }

        CHECK(vc.get_wrap_around_flag() == true);
        CHECK(vc.get_sync_position() == 2);
        CHECK(vc.get_current_position() == 2);
    }
}

TEST_CASE("UniqueVectorContainer Insertion and Uniqueness", "[UniqueVectorContainer]") {

    SECTION("Unique Insertions") {
        UniqueVectorContainer uvc(3, 3); // Container for 3 vectors, each of dimension 3
        std::optional<size_t> pos;

        // Insert (1.0, 2.0, 3.0)
        pos = uvc.insert(std::vector<float>{1.0f, 2.0f, 3.0f});
        REQUIRE(pos.has_value());
        CHECK(pos.value() == 0);

        // Insert (1.0, 2.0, 3.0) again, should return std::nullopt
        pos = uvc.insert(std::vector<float>{1.0f, 2.0f, 3.0f});
        CHECK_FALSE(pos.has_value());

        // Insert (4.0, 5.0, 6.0)
        pos = uvc.insert(std::vector<float>{4.0f, 5.0f, 6.0f});
        REQUIRE(pos.has_value());
        CHECK(pos.value() == 1);

        // Insert (4.0, 6.0, 5.0)
        pos = uvc.insert(std::vector<float>{4.0f, 6.0f, 5.0f});
        REQUIRE(pos.has_value());
        CHECK(pos.value() == 2);

        // Check contents of container
        CHECK(uvc.get(0) == std::vector<float>{1.0f, 2.0f, 3.0f});
        CHECK(uvc.get(1) == std::vector<float>{4.0f, 5.0f, 6.0f});
        CHECK(uvc.get(2) == std::vector<float>{4.0f, 6.0f, 5.0f});
    }

    SECTION("Handling Container Capacity and Wrap-Around") {
        // Assuming container now contains (1 2 3), (4 5 6), (4 6 5)
        UniqueVectorContainer uvc(3, 3); // Container for 3 vectors, each of dimension 3
        uvc.insert(std::vector<float>{1.0f, 2.0f, 3.0f});
        uvc.insert(std::vector<float>{4.0f, 5.0f, 6.0f});
        uvc.insert(std::vector<float>{4.0f, 6.0f, 5.0f});

        // Insert (11, 12, 13), expect wrap-around and return 0
        auto pos = uvc.insert(std::vector<float>{11.0f, 12.0f, 13.0f});
        REQUIRE(pos.has_value());
        CHECK(pos.value() == 0);

        // Insert (1.0, 2.0, 3.0), should return 1
        pos = uvc.insert(std::vector<float>{1.0f, 2.0f, 3.0f});
        REQUIRE(pos.has_value());
        CHECK(pos.value() == 1);

        // Container should now contain (11 12 13), (1 2 3), (4 6 5)
        CHECK(uvc.get(0) == std::vector<float>{11.0f, 12.0f, 13.0f});
        CHECK(uvc.get(1) == std::vector<float>{1.0f, 2.0f, 3.0f});
        CHECK(uvc.get(2) == std::vector<float>{4.0f, 6.0f, 5.0f});

        // Insert (4 6 5), should return std::nullopt
        pos = uvc.insert(std::vector<float>{4.0f, 6.0f, 5.0f});
        CHECK_FALSE(pos.has_value());

        // Insert (11 12 13), should also return std::nullopt
        pos = uvc.insert(std::vector<float>{11.0f, 12.0f, 13.0f});
        CHECK_FALSE(pos.has_value());
    }
}
