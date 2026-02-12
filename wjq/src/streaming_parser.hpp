#pragma once
#include "callbacks.hpp"
#include "json_parser.hpp"
#include "printer.hpp"
#include "themes.hpp"
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <simdjson.h>
#include <string>
#include <string_view>
#include <vector>

namespace colored_json {

// Configuration for streaming parser
struct StreamingConfig {
  size_t buffer_size = 64 * 1024;         // 64KB default buffer
  size_t max_string_length = 1024 * 1024; // 1MB max string
  bool validate_utf8 = true;
};

// Simple line-by-line JSONL parser (most memory efficient)
class LineByLineParser {
public:
  using DocumentHandler = std::function<bool(const std::string &line, size_t line_number,
                                              const CallbackRegistry *callbacks)>;

  explicit LineByLineParser(size_t buffer_size = 64 * 1024)
      : buffer_size_(buffer_size) {}

  // Parse file line by line
  bool parse_file(const std::string &filename, DocumentHandler handler,
                  const CallbackRegistry *callbacks = nullptr) {
    stats_ = Stats{};

    std::ifstream file(filename);
    if (!file.is_open()) {
      last_error_ = "Cannot open file: " + filename;
      return false;
    }

    return parse_stream(file, std::move(handler), callbacks);
  }

  // Parse stream line by line
  bool parse_stream(std::istream &input, DocumentHandler handler,
                    const CallbackRegistry *callbacks = nullptr) {
    stats_ = Stats{};
    auto start_time = std::chrono::steady_clock::now();

    std::string line;
    std::vector<char> buffer;
    buffer.reserve(buffer_size_);

    while (std::getline(input, line)) {
      stats_.lines_read++;
      stats_.bytes_read += line.size();

      // Trim whitespace
      size_t start = line.find_first_not_of(" \t\r\n");
      if (start == std::string::npos)
        continue;

      size_t end = line.find_last_not_of(" \t\r\n");
      std::string trimmed = line.substr(start, end - start + 1);

      if (trimmed.empty())
        continue;

      if (!handler(trimmed, stats_.lines_read, callbacks)) {
        break;
      }

      stats_.lines_parsed++;

      // Report progress periodically
      if (callbacks && callbacks->has_progress_callback() &&
          stats_.lines_read % 1000 == 0) {
        callbacks->invoke_progress(stats_.bytes_read, 0, "");
      }
    }

    auto end_time = std::chrono::steady_clock::now();
    stats_.parse_time_ms =
        std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return true;
  }

  // Get last error
  std::string last_error() const { return last_error_; }

  struct Stats {
    size_t lines_read = 0;
    size_t lines_parsed = 0;
    size_t bytes_read = 0;
    double parse_time_ms = 0;
  };

  Stats stats() const { return stats_; }

private:
  size_t buffer_size_;
  std::string last_error_;
  Stats stats_;
};

// Streaming JSON printer - processes documents as they come
class StreamingPrinter {
public:
  StreamingPrinter(const Style &style, const CallbackRegistry *callbacks = nullptr)
      : style_(style), callbacks_(callbacks) {}

  // Print a single line of JSON
  bool print_line(const std::string &line, std::ostream &output = std::cout) {
    try {
      // Use the printFromJson interface which handles everything internally
      Printer printer(style_, callbacks_);
      printer.printFromJson(line);
      output << printer.str() << "\n";

      return true;
    } catch (const std::exception &e) {
      last_error_ = e.what();
      return false;
    }
  }

  // Process file line by line
  bool process_file(const std::string &filename, std::ostream &output = std::cout) {
    LineByLineParser parser;

    return parser.parse_file(filename,
      [&](const std::string &line, size_t line_num, const CallbackRegistry *cb) -> bool {
        (void)line_num;
        (void)cb;
        return print_line(line, output);
      }, callbacks_);
  }

  // Process stream line by line
  bool process_stream(std::istream &input, std::ostream &output = std::cout) {
    LineByLineParser parser;

    return parser.parse_stream(input,
      [&](const std::string &line, size_t line_num, const CallbackRegistry *cb) -> bool {
        (void)line_num;
        (void)cb;
        return print_line(line, output);
      }, callbacks_);
  }

  std::string last_error() const { return last_error_; }

private:
  Style style_;
  const CallbackRegistry *callbacks_;
  std::string last_error_;
};

// Simple file parser for larger files
class StreamingJsonParser {
public:
  explicit StreamingJsonParser(const StreamingConfig &config = {})
      : config_(config) {}

  // Parse JSONL file efficiently
  bool parse_jsonl_file(const std::string &filename,
                        std::function<void(const std::string &)> line_handler,
                        const CallbackRegistry *callbacks = nullptr) {
    last_error_.clear();
    stats_ = Stats{};

    std::ifstream file(filename);
    if (!file.is_open()) {
      last_error_ = "Cannot open file: " + filename;
      return false;
    }

    return parse_jsonl_stream(file, std::move(line_handler), callbacks);
  }

  // Parse JSONL stream
  bool parse_jsonl_stream(std::istream &input,
                          std::function<void(const std::string &)> line_handler,
                          const CallbackRegistry *callbacks = nullptr) {
    last_error_.clear();
    stats_ = Stats{};
    auto start_time = std::chrono::steady_clock::now();

    std::string line;
    size_t line_num = 0;

    while (std::getline(input, line)) {
      line_num++;
      stats_.bytes_processed += line.size();

      // Skip empty lines
      if (line.find_first_not_of(" \t\r\n") == std::string::npos)
        continue;

      // Call handler
      line_handler(line);
      stats_.documents_parsed++;

      // Progress callback
      if (callbacks && callbacks->has_progress_callback() &&
          line_num % 1000 == 0) {
        callbacks->invoke_progress(stats_.bytes_processed, 0, "");
      }
    }

    auto end_time = std::chrono::steady_clock::now();
    stats_.parse_time_ms =
        std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return true;
  }

  // Simple file reading for regular JSON
  std::string read_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
      last_error_ = "Cannot open file: " + filename;
      return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    stats_.bytes_processed = buffer.str().size();
    return buffer.str();
  }

  std::string last_error() const { return last_error_; }

  struct Stats {
    size_t bytes_processed = 0;
    size_t documents_parsed = 0;
    double parse_time_ms = 0;
  };

  Stats stats() const { return stats_; }

private:
  StreamingConfig config_;
  std::string last_error_;
  Stats stats_;
};

} // namespace colored_json
