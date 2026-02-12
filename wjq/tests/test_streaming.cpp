#include <catch2/catch_test_macros.hpp>
#include "streaming_parser.hpp"
#include "printer.hpp"
#include "themes.hpp"
#include <sstream>
#include <fstream>

using namespace colored_json;

TEST_CASE("StreamingConfig defaults", "[streaming]") {
    StreamingConfig config;

    REQUIRE(config.buffer_size == 64 * 1024);
    REQUIRE(config.max_string_length == 1024 * 1024);
    REQUIRE(config.validate_utf8 == true);
}

TEST_CASE("LineByLineParser construction", "[streaming]") {
    LineByLineParser parser;
    REQUIRE(parser.last_error().empty());
}

TEST_CASE("LineByLineParser parse stream", "[streaming]") {
    std::string jsonl = R"({"id": 1}
{"id": 2})";
    std::istringstream stream(jsonl);

    LineByLineParser parser;

    int line_count = 0;
    auto handler = [&](const std::string &line, size_t line_num, const CallbackRegistry *) -> bool {
        (void)line;
        (void)line_num;
        line_count++;
        return true;
    };

    bool success = parser.parse_stream(stream, handler);
    REQUIRE(success == true);
    REQUIRE(line_count == 2);
}

TEST_CASE("LineByLineParser stats", "[streaming]") {
    std::string jsonl = "{\"a\": 1}\n{\"b\": 2}\n";
    std::istringstream stream(jsonl);

    LineByLineParser parser;

    auto handler = [&](const std::string &, size_t, const CallbackRegistry *) -> bool {
        return true;
    };

    parser.parse_stream(stream, handler);

    auto stats = parser.stats();
    REQUIRE(stats.lines_read == 2);
    REQUIRE(stats.lines_parsed == 2);
    REQUIRE(stats.parse_time_ms >= 0);
}

TEST_CASE("LineByLineParser early termination", "[streaming]") {
    std::string jsonl = "{\"a\": 1}\n{\"b\": 2}\n{\"c\": 3}";
    std::istringstream stream(jsonl);

    LineByLineParser parser;

    int line_count = 0;
    auto handler = [&](const std::string &, size_t, const CallbackRegistry *) -> bool {
        line_count++;
        return line_count < 2; // Stop after second line
    };

    bool success = parser.parse_stream(stream, handler);
    REQUIRE(success == true);
    REQUIRE(line_count == 2);
}

TEST_CASE("StreamingPrinter construction", "[streaming]") {
    Style s;
    StreamingPrinter printer(s);
    REQUIRE(printer.last_error().empty());
}

TEST_CASE("StreamingPrinter print line", "[streaming]") {
    Style s;
    s.color_mode = ColorMode::Disabled;
    s.compact = true;

    StreamingPrinter printer(s);

    std::ostringstream output;
    bool success = printer.print_line(R"({"name": "test"})", output);

    REQUIRE(success == true);
    std::string result = output.str();
    REQUIRE(result.find("name") != std::string::npos);
    REQUIRE(result.find("test") != std::string::npos);
}

TEST_CASE("StreamingPrinter invalid JSON", "[streaming]") {
    Style s;
    StreamingPrinter printer(s);

    std::ostringstream output;
    bool success = printer.print_line("not valid json", output);

    REQUIRE(success == false);
}

TEST_CASE("StreamingJsonParser construction", "[streaming]") {
    StreamingConfig config;
    StreamingJsonParser parser(config);

    REQUIRE(parser.last_error().empty());
}

TEST_CASE("StreamingJsonParser parse JSONL stream", "[streaming]") {
    std::string jsonl = "{\"id\": 1}\n{\"id\": 2}\n{\"id\": 3}";
    std::istringstream stream(jsonl);

    StreamingJsonParser parser;

    int count = 0;
    auto handler = [&](const std::string &line) {
        (void)line;
        count++;
    };

    bool success = parser.parse_jsonl_stream(stream, handler);

    REQUIRE(success == true);
    REQUIRE(count == 3);
}

TEST_CASE("StreamingJsonParser stats", "[streaming]") {
    std::string jsonl = "{\"a\": 1}\n{\"b\": 2}";
    std::istringstream stream(jsonl);

    StreamingJsonParser parser;

    auto handler = [&](const std::string &) {};
    parser.parse_jsonl_stream(stream, handler);

    auto stats = parser.stats();
    REQUIRE(stats.documents_parsed == 2);
    REQUIRE(stats.parse_time_ms >= 0);
}

TEST_CASE("Streaming empty stream", "[streaming]") {
    std::istringstream stream("");

    StreamingJsonParser parser;

    int count = 0;
    auto handler = [&](const std::string &) {
        count++;
    };

    bool success = parser.parse_jsonl_stream(stream, handler);
    REQUIRE(success == true);
    REQUIRE(count == 0);
}

TEST_CASE("Streaming whitespace only", "[streaming]") {
    std::istringstream stream("   \n\t\n  ");

    StreamingJsonParser parser;

    int count = 0;
    auto handler = [&](const std::string &) {
        count++;
    };

    bool success = parser.parse_jsonl_stream(stream, handler);
    REQUIRE(success == true);
    REQUIRE(count == 0);
}

TEST_CASE("Integration: streaming with callbacks", "[streaming]") {
    std::string json = R"({"password": "secret123"})";

    Style s;
    s.color_mode = ColorMode::Disabled;

    CallbackRegistry callbacks;
    bool key_received = false;

    callbacks.on_key("password*", [&](const JsonEvent &) {
        key_received = true;
        return CallbackResult{};
    });

    StreamingPrinter printer(s, &callbacks);

    std::ostringstream output;
    printer.print_line(json, output);

    REQUIRE(key_received == true);
}
