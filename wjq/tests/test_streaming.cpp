#include "streaming_parser.hpp"
#include <catch2/catch_test_macros.hpp>
#include <sstream>

using namespace colored_json;

TEST_CASE("StreamingPrinter construction", "[streaming]") {
  Style s;
  StreamingPrinter printer(s);
  REQUIRE(printer.last_error().empty());
}

TEST_CASE("StreamingPrinter process_stream", "[streaming]") {
  Style s;
  s.color_mode = ColorMode::Disabled;
  s.compact = true;

  StreamingPrinter printer(s);

  std::string jsonl = R"({"name": "test1"}
{"name": "test2"})";
  std::istringstream stream(jsonl);
  std::ostringstream output;

  bool success = printer.process_stream(stream, output);

  REQUIRE(success == true);
  std::string result = output.str();
  REQUIRE(result.find("test1") != std::string::npos);
  REQUIRE(result.find("test2") != std::string::npos);
  REQUIRE(printer.stats().documents_parsed == 2);
}

TEST_CASE("StreamingPrinter multi-line document", "[streaming]") {
  Style s;
  s.color_mode = ColorMode::Disabled;
  s.compact = true;

  StreamingPrinter printer(s);

  // Multi-line JSON object (would break older LineByLineParser)
  std::string json = R"({
        "name": "multiline",
        "value": 42
    })";

  std::istringstream stream(json);
  std::ostringstream output;

  bool success = printer.process_stream(stream, output);

  REQUIRE(success == true);
  std::string result = output.str();
  REQUIRE(result.find("multiline") != std::string::npos);
  REQUIRE(printer.stats().documents_parsed == 1);
}

TEST_CASE("StreamingPrinter invalid JSON", "[streaming]") {
  Style s;
  StreamingPrinter printer(s);

  std::ostringstream output;
  bool success = printer.print_line("not valid json", output);

  REQUIRE(success == false);
  REQUIRE(!printer.last_error().empty());
}

TEST_CASE("StreamingPrinter empty stream", "[streaming]") {
  std::istringstream stream("");
  StreamingPrinter printer(Style{});
  std::ostringstream output;

  bool success = printer.process_stream(stream, output);
  REQUIRE(success == true);
  REQUIRE(printer.stats().documents_parsed == 0);
}

TEST_CASE("StreamingPrinter whitespace only", "[streaming]") {
  std::istringstream stream("   \n\t\n  ");
  StreamingPrinter printer(Style{});
  std::ostringstream output;

  bool success = printer.process_stream(stream, output);
  REQUIRE(success == true);
  REQUIRE(printer.stats().documents_parsed == 0);
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
