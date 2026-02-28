#include "callbacks.hpp"
#include "json_parser.hpp"
#include "jsonpath.hpp"
#include "printer.hpp"
#include "streaming_parser.hpp"
#include "style.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Windows-specific headers for console colors
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

void enableWindowsAnsiSupport() {
#ifdef _WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)
    return;

  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode))
    return;

  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);
#endif
}

struct CommandLineOptions {
  std::string filter = ".";
  std::string filename;
  std::string theme = "default";
  std::string color_mode_str = "auto";
  bool compact = false;
  int indent_size = 2;
  bool show_help = false;
  bool show_version = false;
  bool show_themes = false;
  bool streaming = false;      // Use streaming parser for large files
  bool line_by_line = false;   // Parse JSONL line by line
  bool hide_sensitive = false; // Hide sensitive fields
  size_t truncate_strings = 0; // Truncate strings longer than this
  bool format_numbers = false; // Add thousand separators

  bool raw_output = false; // -r Output raw strings
  bool slurp = false;      // -s Read stream of objects into array
  std::string tee_file;    // --tee Output raw input to file
  std::map<std::string, std::string> args;     // string args
  std::map<std::string, std::string> argjsons; // json args
};

void printUsage(const char *programName) {
  std::cerr << "Usage: " << programName << " [options] [filter] [file]\n\n";
  std::cerr << "Options:\n";
  std::cerr << "  -t, --theme THEME      Choose color scheme:\n";
  std::cerr
      << "                         default, dracula, solarized, monokai,\n";
  std::cerr
      << "                         github, minimal, neon, ocean, forest,\n";
  std::cerr << "                         cyberpunk, sunset, high-contrast,\n";
  std::cerr << "                         debug, depth-aware, data-analysis,\n";
  std::cerr << "                         white, nord, gruvbox, one-dark,\n";
  std::cerr << "                         catppuccin, ice, coffee\n";
  std::cerr << "  -m, --color-mode MODE  Color mode: auto, 16, 256, truecolor, "
               "disabled\n";
  std::cerr << "  -c, --compact          Compact output (minimal whitespace)\n";
  std::cerr << "  -i, --indent N         Indent size (1-8, default: 2)\n";
  std::cerr
      << "  -S, --streaming        Use streaming parser for large files\n";
  std::cerr << "  -l, --line-by-line     Parse JSONL line by line (memory "
               "efficient)\n";
  std::cerr << "  -r, --raw-output       Output raw strings, not JSON texts\n";
  std::cerr
      << "  -s, --slurp            Read (slurp) all inputs into an array\n";
  std::cerr
      << "  --arg name value       Set variable $name to string <value>\n";
  std::cerr
      << "  --argjson name value   Set variable $name to JSON value <value>\n";
  std::cerr
      << "  --hide-sensitive       Hide sensitive fields (passwords, tokens)\n";
  std::cerr
      << "  --truncate N           Truncate strings longer than N chars\n";
  std::cerr << "  --format-numbers       Add thousand separators to numbers\n";
  std::cerr
      << "  --tee FILE             Output raw, uncolored input data to FILE\n";
  std::cerr << "  --themes               List all available themes\n";
  std::cerr << "  -h, --help             Show this help\n";
  std::cerr << "  -v, --version          Show version\n\n";
  std::cerr << "Filters (jq-style):\n";
  std::cerr << "  .                      Show entire document (default)\n";
  std::cerr << "  .key                   Get property 'key'\n";
  std::cerr << "  .key.nested            Get nested property\n";
  std::cerr << "  .users[]               Iterate array 'users'\n";
  std::cerr << "  .users[].name          Get 'name' from each user\n";
  std::cerr << "  .items[0]              Get first item\n";
  std::cerr << "  .items[-1]             Get last item\n";
  std::cerr << "  .items[0:5]            Get first 5 items\n";
  std::cerr << "  ..name                 Recursive descent for 'name'\n";
  std::cerr << "  keys                   Get object keys\n";
  std::cerr << "  values                 Get object values\n";
  std::cerr << "  length                 Get length of array/string/object\n";
  std::cerr << "  sort                   Sort array\n";
  std::cerr << "  reverse                Reverse array\n";
  std::cerr << "  unique                 Get unique values\n";
  std::cerr << "  map(expr)              Map expression over array\n";
  std::cerr << "  select(condition)      Filter array elements\n";
  std::cerr << "  .items | length        Pipe: get length of items\n";
  std::cerr << "  .users | map(.name)    Get all user names\n";
  std::cerr << "  .users | select(.age > 18)  Filter users by age\n\n";
  std::cerr << "JSONPath (XPath-style):\n";
  std::cerr << "  $.store.book[0]        Get first book\n";
  std::cerr << "  $..book               Get all books recursively\n";
  std::cerr << "  $..book[?(@.price<10)] Filter books by price\n";
  std::cerr << "  $.store.*             Get all store properties\n";
  std::cerr << "  $..author             Get all authors recursively\n\n";
  std::cerr << "Examples:\n";
  std::cerr << "  " << programName << " data.json\n";
  std::cerr << "  " << programName << " -t monokai data.json\n";
  std::cerr << "  " << programName << " '.users[].name' data.json\n";
  std::cerr << "  " << programName << " '.items | length' data.json\n";
  std::cerr << "  " << programName << " '.users | select(.active)' data.json\n";
  std::cerr << "  " << programName << " -t debug -c data.json\n";
  std::cerr << "  " << programName << " -S large-file.json\n";
  std::cerr << "  " << programName
            << " -t data-analysis --format-numbers data.json\n";
  std::cerr << "  cat data.json | " << programName << " -t depth-aware\n";
}

void printVersion() {
  std::cout << "wjq 2.0.0 - Windows JSON Query Tool\n";
  std::cout << "Enhanced with conditional styling, streaming, and callbacks\n";
}

void printThemes() {
  std::cout << "Available themes:\n\n";
  auto themes = colored_json::Style::listPresets();

  std::cout << "Basic themes:\n";
  for (const auto &t : themes) {
    std::cout << "  - " << t << "\n";
  }

  std::cout << "\nSpecial themes:\n";
  std::cout << "  - debug         Highlights errors, warnings, and anomalies\n";
  std::cout << "  - depth-aware   Colors based on nesting depth\n";
  std::cout
      << "  - data-analysis Highlights array patterns (first/last/even/odd)\n";
}

CommandLineOptions parseArguments(int argc, char *argv[]) {
  CommandLineOptions opts;
  std::vector<std::string> positional;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      opts.show_help = true;
      return opts;
    } else if (arg == "-v" || arg == "--version") {
      opts.show_version = true;
      return opts;
    } else if (arg == "--themes") {
      opts.show_themes = true;
      return opts;
    } else if (arg == "-t" || arg == "--theme") {
      if (i + 1 < argc) {
        opts.theme = argv[++i];
      } else {
        throw std::runtime_error("THEME missing after " + arg);
      }
    } else if (arg == "-m" || arg == "--color-mode") {
      if (i + 1 < argc) {
        opts.color_mode_str = argv[++i];
      } else {
        throw std::runtime_error("MODE missing after " + arg);
      }
    } else if (arg == "-c" || arg == "--compact") {
      opts.compact = true;
    } else if (arg == "-i" || arg == "--indent") {
      if (i + 1 < argc) {
        opts.indent_size = std::stoi(argv[++i]);
        if (opts.indent_size < 1 || opts.indent_size > 8) {
          throw std::runtime_error("Indent must be between 1 and 8");
        }
      } else {
        throw std::runtime_error("N missing after " + arg);
      }
    } else if (arg == "-S" || arg == "--streaming") {
      opts.streaming = true;
    } else if (arg == "-s" || arg == "--slurp") {
      opts.slurp = true;
    } else if (arg == "-r" || arg == "--raw-output") {
      opts.raw_output = true;
    } else if (arg == "--arg") {
      if (i + 2 < argc) {
        opts.args[argv[i + 1]] = argv[i + 2];
        i += 2;
      } else {
        throw std::runtime_error("VAR and VALUE missing after " + arg);
      }
    } else if (arg == "--argjson") {
      if (i + 2 < argc) {
        opts.argjsons[argv[i + 1]] = argv[i + 2];
        i += 2;
      } else {
        throw std::runtime_error("VAR and VALUE missing after " + arg);
      }
    } else if (arg == "-l" || arg == "--line-by-line") {
      opts.line_by_line = true;
    } else if (arg == "--hide-sensitive") {
      opts.hide_sensitive = true;
    } else if (arg == "--truncate") {
      if (i + 1 < argc) {
        opts.truncate_strings = std::stoul(argv[++i]);
      } else {
        throw std::runtime_error("N missing after " + arg);
      }
    } else if (arg == "--format-numbers") {
      opts.format_numbers = true;
    } else if (arg == "--tee") {
      if (i + 1 < argc) {
        opts.tee_file = argv[++i];
      } else {
        throw std::runtime_error("FILE missing after " + arg);
      }
    } else if (arg.size() > 0 && arg[0] == '-' && arg != "-") {
      throw std::runtime_error("Unknown option: " + arg);
    } else {
      positional.push_back(arg);
    }
  }

  // Process positional arguments: [filter] [file]
  // When 2+ args: first is always filter, second is file.
  // When 1 arg: heuristic - it's a file if it looks like a path
  //   (contains / or \ or ends with a file extension like .json)
  //   otherwise it's a filter.
  if (positional.size() == 1) {
    const auto &p = positional[0];
    bool looks_like_file = false;
    // Check for path separators or common file extensions
    if (p.find('/') != std::string::npos || p.find('\\') != std::string::npos ||
        (p.size() > 5 && p.substr(p.size() - 5) == ".json") ||
        (p.size() > 6 && p.substr(p.size() - 6) == ".jsonl") ||
        (p.size() > 4 && p.substr(p.size() - 4) == ".txt")) {
      looks_like_file = true;
    }
    // Also: if it starts with . but has a file extension, treat as file
    if (looks_like_file) {
      opts.filename = p;
    } else {
      opts.filter = p;
    }
  } else if (positional.size() >= 2) {
    opts.filter = positional[0];
    opts.filename = positional[1];
  }

  return opts;
}

std::string readInput(const std::string &filename,
                      const std::string &tee_file = "") {
  std::string result;
  if (filename.empty() || filename == "-") {
    std::stringstream buffer;
    buffer << std::cin.rdbuf();
    result = buffer.str();
  } else {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    result = buffer.str();
  }

  if (!tee_file.empty()) {
    std::ofstream tf(tee_file, std::ios::binary);
    if (!tf) {
      throw std::runtime_error("Cannot open tee file for writing: " + tee_file);
    }
    tf.write(result.data(), result.size());
  }

  return result;
}

colored_json::ColorMode parseColorMode(const std::string &mode) {
  static const std::unordered_map<std::string, colored_json::ColorMode>
      modeMap = {{"auto", colored_json::ColorMode::Auto},
                 {"16", colored_json::ColorMode::Ansi16},
                 {"256", colored_json::ColorMode::Ansi256},
                 {"truecolor", colored_json::ColorMode::TrueColor},
                 {"disabled", colored_json::ColorMode::Disabled}};

  auto it = modeMap.find(mode);
  if (it != modeMap.end()) {
    return it->second;
  }
  throw std::runtime_error("Invalid color mode: " + mode);
}

// Setup callbacks based on options
colored_json::CallbackRegistry setupCallbacks(const CommandLineOptions &opts) {
  using namespace colored_json;
  CallbackRegistry callbacks;

  if (opts.hide_sensitive) {
    callbacks.on_key("password*", callbacks::hide_sensitive("***"));
    callbacks.on_key("token*", callbacks::hide_sensitive("***"));
    callbacks.on_key("secret*", callbacks::hide_sensitive("***"));
    callbacks.on_key("api_key", callbacks::hide_sensitive("***"));
    callbacks.on_key("*password", callbacks::hide_sensitive("***"));
    callbacks.on_key("*token", callbacks::hide_sensitive("***"));
  }

  if (opts.truncate_strings > 0) {
    callbacks.on_value_transform(
        callbacks::truncate_strings(opts.truncate_strings));
  }

  if (opts.format_numbers) {
    callbacks.on_value_transform(callbacks::format_numbers());
  }

  return callbacks;
}

// Process JSON using streaming parser
int processStreaming(const std::string &filename,
                     const CommandLineOptions &opts,
                     const colored_json::CallbackRegistry &callbacks) {
  using namespace colored_json;

  Style style = Style::getPreset(opts.theme);
  style.color_mode = parseColorMode(opts.color_mode_str);
  style.compact = opts.compact;
  style.indent_size = opts.indent_size;

  StreamingPrinter printer(style,
                           callbacks.has_callbacks() ? &callbacks : nullptr);
  bool success;

  std::unique_ptr<std::ofstream> tee_stream;
  std::ostream *tee_ptr = nullptr;
  if (!opts.tee_file.empty()) {
    tee_stream =
        std::make_unique<std::ofstream>(opts.tee_file, std::ios::binary);
    if (!*tee_stream) {
      std::cerr << "Error: Cannot open tee file for writing: " << opts.tee_file
                << "\n";
      return 1;
    }
    tee_ptr = tee_stream.get();
  }

  if (filename.empty() || filename == "-") {
    success = printer.process_stream(std::cin, std::cout, tee_ptr);
  } else {
    success = printer.process_file(filename, std::cout, tee_ptr);
  }

  if (!success) {
    std::cerr << "Error: " << printer.last_error() << "\n";
    return 1;
  }

  return 0;
}

// Build variables map from CLI --arg and --argjson
std::map<std::string, colored_json::QueryValue>
buildVarsMap(const CommandLineOptions &opts) {
  std::map<std::string, colored_json::QueryValue> vars;
  for (const auto &kv : opts.args) {
    vars[kv.first] = colored_json::QueryValue(kv.second);
  }
  for (const auto &kv : opts.argjsons) {
    // Parse JSON value - try number, bool, null, otherwise treat as string
    const std::string &v = kv.second;
    if (v == "true") {
      vars[kv.first] = colored_json::QueryValue(true);
    } else if (v == "false") {
      vars[kv.first] = colored_json::QueryValue(false);
    } else if (v == "null") {
      vars[kv.first] = colored_json::QueryValue::null();
    } else {
      try {
        double d = std::stod(v);
        vars[kv.first] = colored_json::QueryValue(d);
      } catch (...) {
        vars[kv.first] = colored_json::QueryValue(v);
      }
    }
  }
  return vars;
}

// Strip surrounding quotes from a JSON string for raw output
std::string stripJsonQuotes(const std::string &s) {
  if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
    std::string result;
    // Unescape JSON string
    for (size_t i = 1; i < s.size() - 1; ++i) {
      if (s[i] == '\\' && i + 1 < s.size() - 1) {
        switch (s[i + 1]) {
        case '"':
          result += '"';
          ++i;
          break;
        case '\\':
          result += '\\';
          ++i;
          break;
        case 'n':
          result += '\n';
          ++i;
          break;
        case 't':
          result += '\t';
          ++i;
          break;
        case 'r':
          result += '\r';
          ++i;
          break;
        default:
          result += s[i];
          break;
        }
      } else {
        result += s[i];
      }
    }
    return result;
  }
  return s;
}

// Execute jq-style or JSONPath query filter
int executeQueryFilter(const std::string &input, const CommandLineOptions &opts,
                       const colored_json::CallbackRegistry &) {
  using namespace colored_json;

  if (input.empty()) {
    std::cerr << "No JSON input received (stdin was empty or file is empty)\n";
    return 1;
  }

  auto vars = buildVarsMap(opts);

  // Check if this is a JSONPath query (starts with $)
  if (JsonQuery::is_jsonpath(opts.filter)) {
    std::string result = JsonQuery::execute(input, opts.filter, vars);
    if (!JsonQuery::get_error().empty()) {
      std::cerr << "JSONPath error: " << JsonQuery::get_error() << "\n";
      return 1;
    }
    if (opts.raw_output) {
      std::cout << stripJsonQuotes(result) << "\n";
    } else {
      std::cout << result << "\n";
    }
    return 0;
  }

  // Execute jq-style query
  std::string result = JsonQuery::execute(input, opts.filter, vars);
  if (!JsonQuery::get_error().empty()) {
    std::cerr << "Query error: " << JsonQuery::get_error() << "\n";
    return 1;
  }

  if (opts.raw_output) {
    std::cout << stripJsonQuotes(result) << "\n";
  } else {
    std::cout << result << "\n";
  }
  return 0;
}

// Process JSON using regular parser (for smaller files)
int processRegular(const std::string &input, const CommandLineOptions &opts,
                   const colored_json::CallbackRegistry &callbacks) {
  using namespace colored_json;

  // If filter is not just ".", use query engine
  if (opts.filter != ".") {
    return executeQueryFilter(input, opts, callbacks);
  }

  if (input.empty()) {
    std::cerr << "No JSON input received (stdin was empty or file is empty)\n";
    return 1;
  }

  // Configure style
  Style style = Style::getPreset(opts.theme);
  style.color_mode = parseColorMode(opts.color_mode_str);
  style.compact = opts.compact;
  style.indent_size = opts.indent_size;

  // Create printer with callbacks
  Printer printer(style, callbacks.has_callbacks() ? &callbacks : nullptr);

  // Use padded_string to ensure lifetime
  simdjson::padded_string padded_json(input);

  // Execute filter (support iterate_many for JSONL)
  JsonParser parser;
  auto stream = parser.parseMany(padded_json);

  bool first_doc = true;
  for (auto doc_res : stream) {
    if (!first_doc)
      std::cout << "\n";
    first_doc = false;

    auto doc = doc_res.value();

    printer.clear();
    printer.printDocument(doc);
    std::cout << printer.str() << std::flush;
  }

  std::cout << "\n";
  return 0;
}

int main(int argc, char *argv[]) {
  try {
    // Parse command line
    CommandLineOptions opts = parseArguments(argc, argv);

    if (opts.show_help) {
      printUsage(argv[0]);
      return 0;
    }

    if (opts.show_version) {
      printVersion();
      return 0;
    }

    if (opts.show_themes) {
      printThemes();
      return 0;
    }

    // Enable Windows ANSI support
    enableWindowsAnsiSupport();

    // Detect if stdin is a pipe/redirect
    bool is_stdin_pipe = false;
#ifdef _WIN32
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
      DWORD type = GetFileType(hIn);
      if (type == FILE_TYPE_PIPE) {
        is_stdin_pipe = true;
      }
    }
#else
    if (!isatty(STDIN_FILENO)) {
      is_stdin_pipe = true;
    }
#endif

    // If reading from stdin and it's a pipe, default to streaming unless slurp
    // is on
    if ((opts.filename.empty() || opts.filename == "-") && is_stdin_pipe &&
        !opts.slurp) {
      opts.streaming = true;
    }

    // Setup callbacks
    colored_json::CallbackRegistry callbacks = setupCallbacks(opts);

    // Process based on mode
    if (opts.streaming || opts.line_by_line) {
      return processStreaming(opts.filename, opts, callbacks);
    } else {
      // Read all input for regular mode
      std::string json_str = readInput(opts.filename, opts.tee_file);

      // Slurp mode: wrap multiple JSON documents into a single array
      if (opts.slurp) {
        // Simple slurp: wrap entire input in array brackets
        // This will make iterate_many produce items that we collect
        simdjson::padded_string padded_json(json_str);
        colored_json::JsonParser parser;
        auto stream = parser.parseMany(padded_json);

        std::string slurped = "[";
        bool first = true;
        for (auto doc_res : stream) {
          if (!first)
            slurped += ",";
          first = false;
          auto doc = doc_res.value();
          slurped += simdjson::to_json_string(doc).value();
        }
        slurped += "]";
        json_str = slurped;
      }

      return processRegular(json_str, opts, callbacks);
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
