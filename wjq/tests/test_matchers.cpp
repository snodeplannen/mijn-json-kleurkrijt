#include <catch2/catch_test_macros.hpp>
#include "matchers.hpp"
#include "style_context.hpp"

using namespace colored_json;

TEST_CASE("KeywordMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("Exact match") {
        auto matcher = keyword({"error", "warning"});
        REQUIRE(matcher->match("error", ctx).matched == true);
        REQUIRE(matcher->match("warning", ctx).matched == true);
        REQUIRE(matcher->match("success", ctx).matched == false);
    }

    SECTION("Case insensitive") {
        auto matcher = keyword({"ERROR"}, false);
        REQUIRE(matcher->match("error", ctx).matched == true);
        REQUIRE(matcher->match("ERROR", ctx).matched == true);
        REQUIRE(matcher->match("Error", ctx).matched == true);
    }

    SECTION("Substring match") {
        auto matcher = keyword({"err"});
        REQUIRE(matcher->match("error", ctx).matched == true);
        REQUIRE(matcher->match("no error", ctx).matched == true);
    }

    SECTION("Describe") {
        auto matcher = keyword({"a", "b"});
        REQUIRE(matcher->describe() == "keyword(\"a\", \"b\")");
    }
}

TEST_CASE("RegexMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("Simple pattern") {
        auto matcher = regex("[0-9]+");
        REQUIRE(matcher->match("123", ctx).matched == true);
        REQUIRE(matcher->match("abc", ctx).matched == false);
    }

    SECTION("Email pattern") {
        auto matcher = regex("[a-z]+@[a-z]+\\.[a-z]+");
        REQUIRE(matcher->match("test@example.com", ctx).matched == true);
        REQUIRE(matcher->match("invalid", ctx).matched == false);
    }

    SECTION("Describe") {
        auto matcher = regex("test");
        REQUIRE(matcher->describe() == "regex(test)");
    }
}

TEST_CASE("RangeMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("Inclusive range") {
        auto matcher = range(0, 100);
        REQUIRE(matcher->match("50", ctx).matched == true);
        REQUIRE(matcher->match("0", ctx).matched == true);
        REQUIRE(matcher->match("100", ctx).matched == true);
        REQUIRE(matcher->match("-1", ctx).matched == false);
        REQUIRE(matcher->match("101", ctx).matched == false);
    }

    SECTION("Non-numeric") {
        auto matcher = range(0, 100);
        REQUIRE(matcher->match("abc", ctx).matched == false);
        REQUIRE(matcher->match("", ctx).matched == false);
    }

    SECTION("Describe") {
        auto matcher = range(0, 100);
        REQUIRE(matcher->describe() == "range([0, 100])");
    }
}

TEST_CASE("CompareMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("Greater than") {
        auto matcher = greater_than(100);
        REQUIRE(matcher->match("101", ctx).matched == true);
        REQUIRE(matcher->match("100", ctx).matched == false);
        REQUIRE(matcher->match("99", ctx).matched == false);
    }

    SECTION("Less than") {
        auto matcher = less_than(100);
        REQUIRE(matcher->match("99", ctx).matched == true);
        REQUIRE(matcher->match("100", ctx).matched == false);
        REQUIRE(matcher->match("101", ctx).matched == false);
    }

    SECTION("Equal to") {
        auto matcher = equal_to(100);
        REQUIRE(matcher->match("100", ctx).matched == true);
        REQUIRE(matcher->match("99", ctx).matched == false);
        REQUIRE(matcher->match("101", ctx).matched == false);
    }

    SECTION("Describe") {
        auto matcher = greater_than(50);
        REQUIRE(matcher->describe() == "> 50");
    }
}

TEST_CASE("PathMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("Simple path") {
        auto matcher = path("user.name");
        REQUIRE(matcher->match("user.name", ctx).matched == true);
        REQUIRE(matcher->match("user.age", ctx).matched == false);
    }

    SECTION("Wildcard path") {
        auto matcher = path("user.*.name");
        REQUIRE(matcher->match("user.0.name", ctx).matched == true);
        REQUIRE(matcher->match("user.1.name", ctx).matched == true);
        REQUIRE(matcher->match("user.name", ctx).matched == false);
    }

    SECTION("Describe") {
        auto matcher = path("user.*");
        REQUIRE(matcher->describe() == "path(user.*)");
    }
}

TEST_CASE("AnyMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("Any of matchers") {
        std::vector<std::unique_ptr<Matcher>> matchers;
        matchers.push_back(keyword({"error"}));
        matchers.push_back(keyword({"warning"}));
        auto matcher = any_of(std::move(matchers));

        REQUIRE(matcher->match("error", ctx).matched == true);
        REQUIRE(matcher->match("warning", ctx).matched == true);
        REQUIRE(matcher->match("success", ctx).matched == false);
    }
}

TEST_CASE("AllMatcher", "[matchers]") {
    StyleContext ctx;

    SECTION("All of matchers") {
        std::vector<std::unique_ptr<Matcher>> matchers;
        matchers.push_back(keyword({"error"}));
        // Note: AllMatcher requires ALL matchers to pass
        // Since we can't easily combine keyword with range in a meaningful way,
        // let's use two keywords that would both match
        matchers.push_back(regex("err.*"));
        auto matcher = all_of(std::move(matchers));

        REQUIRE(matcher->match("error", ctx).matched == true);
        REQUIRE(matcher->match("err", ctx).matched == false); // keyword fails
    }
}

TEST_CASE("Matcher clone", "[matchers]") {
    StyleContext ctx;
    auto original = keyword({"test"});
    auto clone = original->clone();

    REQUIRE(original->match("test", ctx).matched == clone->match("test", ctx).matched);
    REQUIRE(original->describe() == clone->describe());
}
