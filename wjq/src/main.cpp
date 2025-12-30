#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "json_parser.hpp"
#include "printer.hpp"
#include "style.hpp"

// Windows-specific headers voor console kleuren
#ifdef _WIN32
#include <windows.h>
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
};

void printUsage(const char *programName) {
  std::cerr << "Gebruik: " << programName << " [opties] [filter] [bestand]\n\n";
  std::cerr << "Opties:\n";
  std::cerr << "  -t, --theme THEMA      Kies kleurenschema (default, dracula, "
               "solarized, monokai, github, minimal, neon)\n";
  std::cerr << "  -m, --color-mode MODE  Kleurmodus: auto, 16, 256, truecolor, "
               "disabled\n";
  std::cerr << "  -c, --compact          Compacte output (geen extra "
               "spaties/nieuwe regels)\n";
  std::cerr << "  -i, --indent N         Indentatiegrootte (standaard: 2)\n";
  std::cerr << "  -h, --help             Toon deze help\n";
  std::cerr << "  -v, --version          Toon versie\n\n";
  std::cerr << "Filters:\n";
  std::cerr
      << "  .                      Toon het hele JSON document (standaard)\n\n";
  std::cerr << "Voorbeelden:\n";
  std::cerr << "  " << programName << " data.json\n";
  std::cerr << "  " << programName << " -t monokai data.json\n";
  std::cerr << "  " << programName << " -c -i 4 data.json\n";
  std::cerr << "  cat data.json | " << programName
            << " -t neon --color-mode 256\n";
}

void printVersion() { std::cout << "wjq 1.0.0 - Windows JSON Query Tool\n"; }

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
    } else if (arg == "-t" || arg == "--theme") {
      if (i + 1 < argc) {
        opts.theme = argv[++i];
      } else {
        throw std::runtime_error("THEMA ontbreekt na " + arg);
      }
    } else if (arg == "-m" || arg == "--color-mode") {
      if (i + 1 < argc) {
        opts.color_mode_str = argv[++i];
      } else {
        throw std::runtime_error("MODE ontbreekt na " + arg);
      }
    } else if (arg == "-c" || arg == "--compact") {
      opts.compact = true;
    } else if (arg == "-i" || arg == "--indent") {
      if (i + 1 < argc) {
        opts.indent_size = std::stoi(argv[++i]);
        if (opts.indent_size < 1 || opts.indent_size > 8) {
          throw std::runtime_error("Indent moet tussen 1 en 8 liggen");
        }
      } else {
        throw std::runtime_error("N ontbreekt na " + arg);
      }
    } else if (arg.size() > 0 && arg[0] == '-' && arg != "-") {
      throw std::runtime_error("Onbekende optie: " + arg);
    } else {
      positional.push_back(arg);
    }
  }

  // Verwerk positionele argumenten: [filter] [bestand]
  if (positional.size() == 1) {
    // Als het begint met . is het waarschijnlijk een filter
    if (positional[0][0] == '.') {
      opts.filter = positional[0];
    } else {
      opts.filename = positional[0];
    }
  } else if (positional.size() >= 2) {
    opts.filter = positional[0];
    opts.filename = positional[1];
  }

  return opts;
}

std::string readInput(const std::string &filename) {
  if (filename.empty() || filename == "-") {
    // Controleer of stdin iets te bieden heeft
    // Op Windows is dit lastiger zonder te blocken, maar voor een CLI tool is
    // blocken ok
    std::stringstream buffer;
    buffer << std::cin.rdbuf();
    return buffer.str();
  } else {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Kan bestand niet openen: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
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
  throw std::runtime_error("Ongeldige kleurmodus: " + mode);
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

    // Enable Windows ANSI support
    enableWindowsAnsiSupport();

    // Read JSON input
    std::string json_str = readInput(opts.filename);
    if (json_str.empty()) {
      std::cerr
          << "Geen JSON input ontvangen (stdin was leeg of bestand is leeg)\n";
      return 1;
    }

    // Configure style
    colored_json::Style style = colored_json::Style::getPreset(opts.theme);
    style.color_mode = parseColorMode(opts.color_mode_str);
    style.compact = opts.compact;
    style.indent_size = opts.indent_size;

    // Create printer
    colored_json::Printer printer(style);

    // Use padded_string to ensure lifetime
    simdjson::padded_string padded_json(json_str);

    // Execute filter (nu ondersteunen we iterate_many voor JSONL)
    colored_json::JsonParser parser;
    auto stream = parser.parseMany(padded_json);

    bool first_doc = true;
    for (auto doc_res : stream) {
      if (!first_doc)
        std::cout << "\n";
      first_doc = false;

      auto doc = doc_res.value();

      if (opts.filter == ".") {
        simdjson::ondemand::value val;
        auto val_error = doc.get_value().get(val);
        if (val_error) {
          throw std::runtime_error("Kan root value niet ophalen");
        }

        simdjson::ondemand::json_type type;
        auto type_error = val.type().get(type);
        if (type_error) {
          throw std::runtime_error("Kan JSON type niet bepalen");
        }

        // Print based on type
        if (type == simdjson::ondemand::json_type::object) {
          simdjson::ondemand::object obj;
          auto obj_error = val.get_object().get(obj);
          if (obj_error) {
            throw std::runtime_error("Kan object niet ophalen");
          }
          printer.printDictJson(obj, "");
        } else if (type == simdjson::ondemand::json_type::array) {
          simdjson::ondemand::array arr;
          auto arr_error = val.get_array().get(arr);
          if (arr_error) {
            throw std::runtime_error("Kan array niet ophalen");
          }
          printer.printListJson(arr, "");
        } else {
          printer.printValueJson(val, "");
        }

        std::cout << printer.str();
        printer.clear();
      } else {
        std::cerr << "Niet-ondersteunde filter: " << opts.filter << "\n";
        std::cerr << "Ondersteunde filters: .\n";
        return 1;
      }
    }

    std::cout << "\n";
    return 0;

  } catch (const std::exception &e) {
    std::cerr << "Fout: " << e.what() << "\n";
    return 1;
  }
}
