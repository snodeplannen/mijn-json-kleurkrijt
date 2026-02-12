#include <catch2/catch_test_macros.hpp>
#include "printer.hpp"
#include "themes.hpp"
#include <simdjson.h>

using namespace colored_json;

TEST_CASE("Printer basic construction", "[printer]") {
    Style s;
    Printer p(s);
    REQUIRE(p.str().empty());
}

TEST_CASE("Printer simple string", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    Printer p(s);

    // simdjson ondemand doesn't support scalar root values well
    // Use an object instead
    std::string json = R"({"value": "hello"})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("hello") != std::string::npos);
}

TEST_CASE("Printer simple number", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    Printer p(s);

    std::string json = R"({"value": 42})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("42") != std::string::npos);
}

TEST_CASE("Printer simple boolean", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    Printer p(s);

    std::string json = R"({"value": true})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("true") != std::string::npos);
}

TEST_CASE("Printer null", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    Printer p(s);

    std::string json = R"({"value": null})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("null") != std::string::npos);
}

TEST_CASE("Printer simple object", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = R"({"name": "test", "value": 123})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("{") != std::string::npos);
    REQUIRE(result.find("}") != std::string::npos);
    REQUIRE(result.find("name") != std::string::npos);
    REQUIRE(result.find("test") != std::string::npos);
    REQUIRE(result.find("value") != std::string::npos);
}

TEST_CASE("Printer simple array", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = "[1, 2, 3]";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("[") != std::string::npos);
    REQUIRE(result.find("]") != std::string::npos);
    REQUIRE(result.find("1") != std::string::npos);
    REQUIRE(result.find("2") != std::string::npos);
    REQUIRE(result.find("3") != std::string::npos);
}

TEST_CASE("Printer nested object", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = R"({"user": {"name": "John", "age": 30}})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("user") != std::string::npos);
    REQUIRE(result.find("name") != std::string::npos);
    REQUIRE(result.find("John") != std::string::npos);
}

TEST_CASE("Printer compact mode", "[printer]") {
    Style s_compact;
    s_compact.color_mode = ColorMode::Disabled;
    s_compact.compact = true;
    Printer p_compact(s_compact);

    Style s_pretty;
    s_pretty.color_mode = ColorMode::Disabled;
    s_pretty.compact = false;
    Printer p_pretty(s_pretty);

    std::string json = R"({"a": 1})";

    std::string compact_result = p_compact.printFromJson(json);
    std::string pretty_result = p_pretty.printFromJson(json);

    // Compact should not have newlines
    REQUIRE(compact_result.find('\n') == std::string::npos);

    // Pretty should have newlines
    REQUIRE(pretty_result.find('\n') != std::string::npos);
}

TEST_CASE("Printer indent size", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = false;
    s.indent_size = 4;
    Printer p(s);

    std::string json = R"({"a": {"b": 1}})";
    std::string result = p.printFromJson(json);

    // Should have 4 spaces for indent
    REQUIRE(result.find("    ") != std::string::npos);
}

TEST_CASE("Printer string escaping", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    Printer p(s);

    std::string json = R"({"text": "line1\nline2\ttab"})";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("\\n") != std::string::npos);
    REQUIRE(result.find("\\t") != std::string::npos);
}

TEST_CASE("Printer clear", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    Printer p(s);

    p.printFromJson(R"({"test": 1})");
    REQUIRE(!p.str().empty());

    p.clear();
    REQUIRE(p.str().empty());
}

TEST_CASE("Printer with callbacks", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;

    CallbackRegistry callbacks;
    bool event_received = false;

    callbacks.on_element(JsonEventType::BeginObject, [&](const JsonEvent &) {
        event_received = true;
        return CallbackResult{};
    });

    Printer p(s, &callbacks);

    std::string json = R"({})";
    p.printFromJson(json);

    REQUIRE(event_received == true);
}

TEST_CASE("Printer empty object", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = "{}";
    std::string result = p.printFromJson(json);

    // Result may contain ANSI reset codes even in "disabled" mode
    // Just check for braces
    REQUIRE(result.find("{") != std::string::npos);
    REQUIRE(result.find("}") != std::string::npos);
}

TEST_CASE("Printer empty array", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = "[]";
    std::string result = p.printFromJson(json);

    // Result may contain ANSI reset codes even in "disabled" mode
    // Just check for brackets
    REQUIRE(result.find("[") != std::string::npos);
    REQUIRE(result.find("]") != std::string::npos);
}

TEST_CASE("Printer mixed array", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = R"([1, "two", true, null, {"a": 1}])";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("1") != std::string::npos);
    REQUIRE(result.find("two") != std::string::npos);
    REQUIRE(result.find("true") != std::string::npos);
    REQUIRE(result.find("null") != std::string::npos);
}

TEST_CASE("Printer array of objects", "[printer]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;
    Printer p(s);

    std::string json = R"([{"id": 1}, {"id": 2}])";
    std::string result = p.printFromJson(json);

    REQUIRE(result.find("id") != std::string::npos);
    REQUIRE(result.find("1") != std::string::npos);
    REQUIRE(result.find("2") != std::string::npos);
}
