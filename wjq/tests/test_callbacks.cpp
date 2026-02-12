#include <catch2/catch_test_macros.hpp>
#include "callbacks.hpp"

using namespace colored_json;

TEST_CASE("CallbackRegistry empty by default", "[callbacks]") {
    CallbackRegistry reg;
    REQUIRE(reg.has_callbacks() == false);
}

TEST_CASE("CallbackRegistry element callbacks", "[callbacks]") {
    CallbackRegistry reg;

    bool called = false;
    reg.on_element(JsonEventType::StringValue, [&](const JsonEvent &) {
        called = true;
        return CallbackResult{};
    });

    REQUIRE(reg.has_callbacks() == true);

    JsonEvent event;
    event.type = JsonEventType::StringValue;
    reg.invoke_element_callbacks(event);

    REQUIRE(called == true);
}

TEST_CASE("CallbackRegistry key callbacks", "[callbacks]") {
    CallbackRegistry reg;

    bool called = false;
    reg.on_key("password", [&](const JsonEvent &) {
        called = true;
        return CallbackResult{};
    });

    JsonEvent event;
    event.type = JsonEventType::Key;
    event.key = "password";

    reg.invoke_element_callbacks(event);
    REQUIRE(called == true);
}

TEST_CASE("CallbackRegistry path callbacks", "[callbacks]") {
    CallbackRegistry reg;

    bool called = false;
    reg.on_path("user.*", [&](const JsonEvent &e) {
        called = true;
        REQUIRE(e.path.find("user.") == 0);
        return CallbackResult{};
    });

    JsonEvent event;
    event.type = JsonEventType::Key;
    event.path = "user.name";
    event.key = "name";

    reg.invoke_element_callbacks(event);
    REQUIRE(called == true);
}

TEST_CASE("CallbackResult skip_element", "[callbacks]") {
    CallbackRegistry reg;

    reg.on_element(JsonEventType::StringValue, [&](const JsonEvent &) {
        CallbackResult r;
        r.skip_element = true;
        return r;
    });

    JsonEvent event;
    event.type = JsonEventType::StringValue;

    auto result = reg.invoke_element_callbacks(event);
    REQUIRE(result.skip_element == true);
}

TEST_CASE("CallbackResult custom color", "[callbacks]") {
    CallbackRegistry reg;

    reg.on_element(JsonEventType::StringValue, [&](const JsonEvent &) {
        CallbackResult r;
        r.custom_color = true;
        r.color = Color(255, 0, 0);
        return r;
    });

    JsonEvent event;
    event.type = JsonEventType::StringValue;

    auto result = reg.invoke_element_callbacks(event);
    REQUIRE(result.custom_color == true);
    REQUIRE(result.color == Color(255, 0, 0));
}

TEST_CASE("CallbackResult replace value", "[callbacks]") {
    CallbackRegistry reg;

    reg.on_element(JsonEventType::StringValue, [&](const JsonEvent &) {
        CallbackResult r;
        r.replace_value = true;
        r.replacement_text = "***";
        return r;
    });

    JsonEvent event;
    event.type = JsonEventType::StringValue;

    auto result = reg.invoke_element_callbacks(event);
    REQUIRE(result.replace_value == true);
    REQUIRE(result.replacement_text == "***");
}

TEST_CASE("Value transform callbacks", "[callbacks]") {
    CallbackRegistry reg;

    reg.on_value_transform([](const std::string &val, const StyleContext &) {
        return val + "_modified";
    });

    StyleContext ctx;
    std::string result = reg.invoke_value_transforms("test", ctx);
    REQUIRE(result == "test_modified");

    // Multiple transforms
    reg.on_value_transform([](const std::string &val, const StyleContext &) {
        return val + "_again";
    });

    result = reg.invoke_value_transforms("test", ctx);
    REQUIRE(result == "test_modified_again");
}

TEST_CASE("Color override callbacks", "[callbacks]") {
    CallbackRegistry reg;

    reg.on_color_override([](const JsonEvent &, const Color &) -> std::optional<Color> {
        return Color(255, 0, 0);
    });

    JsonEvent event;
    auto result = reg.invoke_color_overrides(event, Color(0, 0, 0));
    REQUIRE(result.has_value() == true);
    REQUIRE(result.value() == Color(255, 0, 0));
}

TEST_CASE("Color override no match", "[callbacks]") {
    CallbackRegistry reg;

    // Callback that returns nullopt
    reg.on_color_override([](const JsonEvent &, const Color &) -> std::optional<Color> {
        return std::nullopt;
    });

    JsonEvent event;
    auto result = reg.invoke_color_overrides(event, Color(0, 0, 0));
    REQUIRE(result.has_value() == false);
}

TEST_CASE("Progress callback", "[callbacks]") {
    CallbackRegistry reg;

    size_t progress_called = 0;
    reg.on_progress([&](size_t processed, size_t total, const std::string &) {
        progress_called++;
        REQUIRE(processed == 100);
        REQUIRE(total == 1000);
    });

    reg.invoke_progress(100, 1000, "path");
    REQUIRE(progress_called == 1);
    REQUIRE(reg.has_progress_callback() == true);
}

TEST_CASE("callbacks::hide_sensitive", "[callbacks]") {
    auto cb = callbacks::hide_sensitive("***");

    JsonEvent event;
    auto result = cb(event);

    REQUIRE(result.replace_value == true);
    REQUIRE(result.replacement_text == "\"***\"");
}

TEST_CASE("callbacks::truncate_strings", "[callbacks]") {
    auto cb = callbacks::truncate_strings(5, "...");

    StyleContext ctx;
    std::string short_str = "hi";
    std::string long_str = "hello world";

    REQUIRE(cb(short_str, ctx) == "hi");
    REQUIRE(cb(long_str, ctx) == "hello...");
}

TEST_CASE("callbacks::format_numbers", "[callbacks]") {
    auto cb = callbacks::format_numbers(',');

    StyleContext ctx;

    REQUIRE(cb("1234", ctx) == "1,234");
    REQUIRE(cb("1234567", ctx) == "1,234,567");
    REQUIRE(cb("123", ctx) == "123");
    REQUIRE(cb("abc", ctx) == "abc"); // Non-numeric unchanged
}

TEST_CASE("callbacks::highlight_value", "[callbacks]") {
    auto cb = callbacks::highlight_value("error", Color(255, 0, 0));

    JsonEvent event1;
    event1.type = JsonEventType::StringValue;
    event1.string_value = "error";

    auto result1 = cb(event1, Color(0, 0, 0));
    REQUIRE(result1.has_value() == true);
    REQUIRE(result1.value() == Color(255, 0, 0));

    JsonEvent event2;
    event2.type = JsonEventType::StringValue;
    event2.string_value = "success";

    auto result2 = cb(event2, Color(0, 0, 0));
    REQUIRE(result2.has_value() == false);
}

TEST_CASE("JsonEvent construction", "[callbacks]") {
    JsonEvent event;
    event.type = JsonEventType::StringValue;
    event.path = "user.name";
    event.string_value = "John";
    event.depth = 2;
    event.array_index = -1;

    REQUIRE(event.type == JsonEventType::StringValue);
    REQUIRE(event.path == "user.name");
    REQUIRE(event.string_value == "John");
    REQUIRE(event.depth == 2);
}
