#pragma once
#include "callbacks.hpp"
#include "printer.hpp"
#include "style.hpp"
#include <chrono>
#include <iostream>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4244)
#endif
#include <simdjson.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sstream>
#include <string>

namespace colored_json {

// Streaming JSON printer - processes JSON documents efficiently
class StreamingPrinter {
public:
  StreamingPrinter(const Style &style,
                   const CallbackRegistry *callbacks = nullptr)
      : style_(style), callbacks_(callbacks) {}

  // Process a completely loaded padded string using simdjson NDJSON extension
  bool process_padded_string(simdjson::padded_string &json,
                             std::ostream &output = std::cout) {
    auto start_time = std::chrono::steady_clock::now();
    stats_ = Stats{};

    try {
      simdjson::ondemand::parser parser;
      simdjson::ondemand::document_stream stream;
      auto error = parser.iterate_many(json).get(stream);
      if (error) {
        last_error_ = "Failed to start fast parser: " +
                      std::string(simdjson::error_message(error));
        return false;
      }

      Printer printer(style_, callbacks_);
      bool first_doc = true;

      for (auto doc_res : stream) {
        if (doc_res.error()) {
          last_error_ = "Error parsing document in stream: " +
                        std::string(simdjson::error_message(doc_res.error()));
          return false;
        }

        auto doc = doc_res.value();

        if (!first_doc)
          output << "\n";
        first_doc = false;

        printer.clear();
        printer.printDocument(doc);
        output << printer.str();

        stats_.documents_parsed++;
      }

      auto end_time = std::chrono::steady_clock::now();
      stats_.parse_time_ms =
          std::chrono::duration<double, std::milli>(end_time - start_time)
              .count();
      stats_.bytes_processed = json.size();
      return true;

    } catch (const std::exception &e) {
      last_error_ = e.what();
      return false;
    }
  }

  // Process file directly
  bool process_file(const std::string &filename,
                    std::ostream &output = std::cout) {
    simdjson::padded_string json;
    auto error = simdjson::padded_string::load(filename).get(json);
    if (error) {
      last_error_ = "Cannot open file: " + filename + " (" +
                    simdjson::error_message(error) + ")";
      return false;
    }
    return process_padded_string(json, output);
  }

  // Process stream line-by-line for realtime output
  bool process_stream(std::istream &input, std::ostream &output = std::cout) {
    std::string line;
    bool first = true;
    while (std::getline(input, line)) {
      if (line.find_first_not_of(" \t\r\n") == std::string::npos)
        continue;

      if (!first)
        output << "\n";
      first = false;

      if (!print_line(line, output)) {
        return false;
      }
      output << std::flush;
      stats_.documents_parsed++;
    }
    return true;
  }

  // Print a single json string
  bool print_line(const std::string &line, std::ostream &output = std::cout) {
    simdjson::padded_string json(line);
    return process_padded_string(json, output);
  }

  std::string last_error() const { return last_error_; }

  struct Stats {
    size_t bytes_processed = 0;
    size_t documents_parsed = 0;
    double parse_time_ms = 0;
  };
  Stats stats() const { return stats_; }

private:
  Style style_;
  const CallbackRegistry *callbacks_;
  std::string last_error_;
  Stats stats_;
};

} // namespace colored_json
