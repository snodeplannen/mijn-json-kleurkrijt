#pragma once
#include <simdjson.h>
#include <stdexcept>
#include <string>

namespace colored_json {

class JsonParser {
private:
  simdjson::ondemand::parser parser;

public:
  // Parse JSON string en retourneer document
  simdjson::ondemand::document parse(simdjson::padded_string &json) {
    return parser.iterate(json);
  }

  // Parse JSON string met meerdere documenten (JSONL)
  simdjson::ondemand::document_stream parseMany(simdjson::padded_string &json) {
    return parser.iterate_many(json);
  }

  // Parse JSON string en retourneer value (voor root object)
  simdjson::ondemand::value parseValue(simdjson::padded_string &json) {
    auto doc = parse(json);
    return doc.get_value();
  }

  // Check of JSON string geldig is (zonder exception)
  bool isValid(const std::string &json_str) {
    try {
      simdjson::padded_string json(json_str);
      simdjson::ondemand::document doc = parser.iterate(json);
      doc.get_value(); // Probeer value te krijgen
      return true;
    } catch (...) {
      return false;
    }
  }
};

// Helper functie om simdjson error te converteren naar runtime_error
inline std::runtime_error jsonError(const std::string &json_str,
                                    const std::string &error_msg) {
  return std::runtime_error("JSON parsing error: " + error_msg +
                            " (input: " + json_str.substr(0, 100) + "...)");
}

} // namespace colored_json
