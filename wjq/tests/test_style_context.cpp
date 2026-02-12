#include <catch2/catch_test_macros.hpp>
#include "style_context.hpp"

using namespace colored_json;

TEST_CASE("StyleContext basic construction", "[style_context]") {
    StyleContext ctx;

    REQUIRE(ctx.path.empty());
    REQUIRE(ctx.depth == 0);
    REQUIRE(ctx.array_index == -1);
    REQUIRE(ctx.array_length == -1);
    REQUIRE(ctx.parent_type == StyleContext::ParentType::None);
}

TEST_CASE("StyleContext enter_object", "[style_context]") {
    StyleContext root;

    auto child = root.enter_object("user");
    REQUIRE(child.depth == 1);
    REQUIRE(child.key_name == "user");
    REQUIRE(child.path == "user");
    REQUIRE(child.parent_type == StyleContext::ParentType::Object);
    REQUIRE(child.path_components.size() == 1);
    REQUIRE(child.path_components[0] == "user");

    auto grandchild = child.enter_object("name");
    REQUIRE(grandchild.depth == 2);
    REQUIRE(grandchild.path == "user.name");
    REQUIRE(grandchild.path_components.size() == 2);
}

TEST_CASE("StyleContext enter_array", "[style_context]") {
    StyleContext root;

    auto item0 = root.enter_array(0, 5);
    REQUIRE(item0.depth == 1);
    REQUIRE(item0.array_index == 0);
    REQUIRE(item0.array_length == 5);
    REQUIRE(item0.path == "[0]");
    REQUIRE(item0.parent_type == StyleContext::ParentType::Array);

    auto item1 = root.enter_array(1, 5);
    REQUIRE(item1.array_index == 1);
    REQUIRE(item1.path == "[1]");
}

TEST_CASE("StyleContext path_matches", "[style_context]") {
    StyleContext ctx;
    ctx.path = "user.name.first";

    REQUIRE(ctx.path_matches("*") == true);
    REQUIRE(ctx.path_matches("user.name.first") == true);
    REQUIRE(ctx.path_matches("user.name") == false);

    SECTION("Wildcard matching") {
        REQUIRE(ctx.path_matches("user.*") == true);
        REQUIRE(ctx.path_matches("user.*.first") == true);
        REQUIRE(ctx.path_matches("*.name.first") == true);
        REQUIRE(ctx.path_matches("other.*") == false);
    }
}

TEST_CASE("StyleContext depth_in_range", "[style_context]") {
    StyleContext ctx;
    ctx.depth = 3;

    REQUIRE(ctx.depth_in_range(0) == true);
    REQUIRE(ctx.depth_in_range(3) == true);
    REQUIRE(ctx.depth_in_range(0, 3) == true);
    REQUIRE(ctx.depth_in_range(4) == false);
    REQUIRE(ctx.depth_in_range(0, 2) == false);
}

TEST_CASE("StyleContext array helpers", "[style_context]") {
    StyleContext ctx;
    ctx.array_index = 0;
    ctx.array_length = 5;

    SECTION("First item") {
        REQUIRE(ctx.is_first_array_item() == true);
        ctx.array_index = 1;
        REQUIRE(ctx.is_first_array_item() == false);
    }

    SECTION("Last item") {
        ctx.array_index = 4;
        REQUIRE(ctx.is_last_array_item() == true);
        ctx.array_index = 3;
        REQUIRE(ctx.is_last_array_item() == false);
    }

    SECTION("Index in range") {
        ctx.array_index = 2;
        REQUIRE(ctx.array_index_in_range(0, 4) == true);  // 2 in [0, 4]
        REQUIRE(ctx.array_index_in_range(3) == false);    // 2 < 3
        REQUIRE(ctx.array_index_in_range(0, 2) == true);  // 2 in [0, 2] (inclusive)
        REQUIRE(ctx.array_index_in_range(0, 1) == false); // 2 > 1
    }
}

TEST_CASE("StyleContext array_index_matches", "[style_context]") {
    StyleContext ctx;

    SECTION("Even indices") {
        ctx.array_index = 0;
        REQUIRE(ctx.array_index_matches("even") == true);
        ctx.array_index = 1;
        REQUIRE(ctx.array_index_matches("even") == false);
        ctx.array_index = 2;
        REQUIRE(ctx.array_index_matches("even") == true);
    }

    SECTION("Odd indices") {
        ctx.array_index = 0;
        REQUIRE(ctx.array_index_matches("odd") == false);
        ctx.array_index = 1;
        REQUIRE(ctx.array_index_matches("odd") == true);
        ctx.array_index = 3;
        REQUIRE(ctx.array_index_matches("odd") == true);
    }

    SECTION("First/Last") {
        ctx.array_index = 0;
        ctx.array_length = 5;
        REQUIRE(ctx.array_index_matches("first") == true);
        ctx.array_index = 4;
        REQUIRE(ctx.array_index_matches("last") == true);
        ctx.array_index = 2;
        REQUIRE(ctx.array_index_matches("first") == false);
        REQUIRE(ctx.array_index_matches("last") == false);
    }

    SECTION("Every N") {
        ctx.array_index = 0;
        REQUIRE(ctx.array_index_matches("every:3") == true);
        ctx.array_index = 3;
        REQUIRE(ctx.array_index_matches("every:3") == true);
        ctx.array_index = 6;
        REQUIRE(ctx.array_index_matches("every:3") == true);
        ctx.array_index = 1;
        REQUIRE(ctx.array_index_matches("every:3") == false);
    }

    SECTION("Invalid array index") {
        ctx.array_index = -1;
        REQUIRE(ctx.array_index_matches("even") == false);
    }
}
