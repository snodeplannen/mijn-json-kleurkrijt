#pragma once
#include <simdjson.h>
#include <string>
#include <stdexcept>

namespace colored_json {

class JsonParser {
private:
    simdjson::ondemand::parser parser;
    
public:
    // Parse JSON string en retourneer document
    simdjson::ondemand::document parse(const std::string& json_str) {
        simdjson::padded_string json(json_str);
        simdjson::ondemand::document doc = parser.iterate(json);
        return doc;
    }
    
    // Parse JSON string en retourneer value (voor root object)
    simdjson::ondemand::value parseValue(const std::string& json_str) {
        auto doc = parse(json_str);
        return doc.get_value();
    }
    
    // Check of JSON string geldig is (zonder exception)
    bool isValid(const std::string& json_str) {
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
inline std::runtime_error jsonError(const std::string& json_str, const std::string& error_msg) {
    return std::runtime_error("JSON parsing error: " + error_msg + 
                             " (input: " + json_str.substr(0, 100) + "...)");
}

} // namespace colored_json

