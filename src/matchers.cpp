#include "matchers.hpp"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>

namespace colored_json {

// Helper: convert string to lowercase
static std::string to_lower(const std::string &s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// Helper: try to parse double from string
static bool try_parse_double(const std::string &s, double &out) {
  try {
    size_t pos = 0;
    out = std::stod(s, &pos);
    // Check if entire string was consumed
    return pos == s.length();
  } catch (...) {
    return false;
  }
}

// KeywordMatcher implementation
MatchResult KeywordMatcher::match(const std::string &value,
                                   const StyleContext &ctx) const {
  (void)ctx; // Unused for now

  std::string check_value = case_sensitive ? value : to_lower(value);

  for (const auto &kw : keywords) {
    std::string check_kw = case_sensitive ? kw : to_lower(kw);
    if (check_value == check_kw) {
      return MatchResult(true, 1.0, kw);
    }
    // Also check for substring match
    if (check_value.find(check_kw) != std::string::npos) {
      return MatchResult(true, 0.8, kw);
    }
  }
  return MatchResult(false);
}

std::string KeywordMatcher::describe() const {
  std::ostringstream oss;
  oss << "keyword(";
  for (size_t i = 0; i < keywords.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << "\"" << keywords[i] << "\"";
  }
  oss << ")";
  return oss.str();
}

// RegexMatcher implementation
RegexMatcher::RegexMatcher(const std::string &pattern_str)
    : pattern(pattern_str, std::regex::ECMAScript),
      pattern_str(pattern_str) {}

MatchResult RegexMatcher::match(const std::string &value,
                                 const StyleContext &ctx) const {
  (void)ctx;

  std::smatch match;
  if (std::regex_search(value, match, pattern)) {
    return MatchResult(true, 1.0, match[0].str());
  }
  return MatchResult(false);
}

std::string RegexMatcher::describe() const {
  return "regex(" + pattern_str + ")";
}

// RangeMatcher implementation
MatchResult RangeMatcher::match(const std::string &value,
                                 const StyleContext &ctx) const {
  (void)ctx;

  double num;
  if (!try_parse_double(value, num)) {
    return MatchResult(false);
  }

  bool above_min = inclusive_min ? (num >= min_val) : (num > min_val);
  bool below_max = inclusive_max ? (num <= max_val) : (num < max_val);

  if (above_min && below_max) {
    // Calculate how centered in the range (for strength)
    double center = (min_val + max_val) / 2.0;
    double range = max_val - min_val;
    double dist_from_center = std::abs(num - center);
    double strength = 1.0 - (dist_from_center / (range / 2.0));
    strength = std::max(0.5, strength);
    return MatchResult(true, strength);
  }
  return MatchResult(false);
}

std::string RangeMatcher::describe() const {
  std::ostringstream oss;
  oss << "range(";
  oss << (inclusive_min ? "[" : "(");
  oss << min_val << ", " << max_val;
  oss << (inclusive_max ? "]" : ")");
  oss << ")";
  return oss.str();
}

// CompareMatcher implementation
MatchResult CompareMatcher::match(const std::string &value,
                                   const StyleContext &ctx) const {
  (void)ctx;

  double num;
  if (!try_parse_double(value, num)) {
    return MatchResult(false);
  }

  bool result = false;
  switch (op) {
  case CompareOp::Equal:
    result = (num == compare_val);
    break;
  case CompareOp::NotEqual:
    result = (num != compare_val);
    break;
  case CompareOp::Less:
    result = (num < compare_val);
    break;
  case CompareOp::LessEqual:
    result = (num <= compare_val);
    break;
  case CompareOp::Greater:
    result = (num > compare_val);
    break;
  case CompareOp::GreaterEqual:
    result = (num >= compare_val);
    break;
  }

  if (result) {
    // Calculate strength based on distance from comparison value
    double diff = std::abs(num - compare_val);
    double strength = std::max(0.5, 1.0 - (diff / (std::abs(compare_val) + 1.0)));
    return MatchResult(true, strength);
  }
  return MatchResult(false);
}

std::string CompareMatcher::describe() const {
  std::ostringstream oss;
  switch (op) {
  case CompareOp::Equal:
    oss << "==";
    break;
  case CompareOp::NotEqual:
    oss << "!=";
    break;
  case CompareOp::Less:
    oss << "<";
    break;
  case CompareOp::LessEqual:
    oss << "<=";
    break;
  case CompareOp::Greater:
    oss << ">";
    break;
  case CompareOp::GreaterEqual:
    oss << ">=";
    break;
  }
  oss << " " << compare_val;
  return oss.str();
}

// PathMatcher implementation
PathMatcher::PathMatcher(std::string path_pattern) : pattern(std::move(path_pattern)) {
  // Convert glob-like pattern to regex
  // * matches any single key
  // ** matches any path
  // . is the separator
  std::string regex_str;
  regex_str.reserve(pattern.length() * 2);
  regex_str += "^";

  for (size_t i = 0; i < pattern.length(); ++i) {
    char c = pattern[i];
    if (c == '*') {
      if (i + 1 < pattern.length() && pattern[i + 1] == '*') {
        // ** matches any depth
        regex_str += ".*";
        i++; // Skip second *
      } else {
        // * matches single segment
        regex_str += "[^.]*";
      }
    } else if (c == '.') {
      regex_str += "\\.";
    } else {
      regex_str += c;
    }
  }
  regex_str += "$";

  path_regex = std::regex(regex_str, std::regex::ECMAScript);
}

MatchResult PathMatcher::match(const std::string &value,
                                const StyleContext &ctx) const {
  (void)ctx;

  std::smatch match;
  if (std::regex_match(value, match, path_regex)) {
    return MatchResult(true, 1.0, pattern);
  }
  return MatchResult(false);
}

std::string PathMatcher::describe() const { return "path(" + pattern + ")"; }

// AnyMatcher implementation
MatchResult AnyMatcher::match(const std::string &value,
                               const StyleContext &ctx) const {
  for (const auto &m : matchers) {
    auto result = m->match(value, ctx);
    if (result.matched) {
      return result;
    }
  }
  return MatchResult(false);
}

std::string AnyMatcher::describe() const {
  std::ostringstream oss;
  oss << "any_of(";
  for (size_t i = 0; i < matchers.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << matchers[i]->describe();
  }
  oss << ")";
  return oss.str();
}

std::unique_ptr<Matcher> AnyMatcher::clone() const {
  std::vector<std::unique_ptr<Matcher>> cloned;
  for (const auto &m : matchers) {
    cloned.push_back(m->clone());
  }
  return std::make_unique<AnyMatcher>(std::move(cloned));
}

// AllMatcher implementation
MatchResult AllMatcher::match(const std::string &value,
                               const StyleContext &ctx) const {
  double min_strength = 1.0;
  for (const auto &m : matchers) {
    auto result = m->match(value, ctx);
    if (!result.matched) {
      return MatchResult(false);
    }
    min_strength = std::min(min_strength, result.match_strength);
  }
  return MatchResult(true, min_strength);
}

std::string AllMatcher::describe() const {
  std::ostringstream oss;
  oss << "all_of(";
  for (size_t i = 0; i < matchers.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << matchers[i]->describe();
  }
  oss << ")";
  return oss.str();
}

std::unique_ptr<Matcher> AllMatcher::clone() const {
  std::vector<std::unique_ptr<Matcher>> cloned;
  for (const auto &m : matchers) {
    cloned.push_back(m->clone());
  }
  return std::make_unique<AllMatcher>(std::move(cloned));
}

} // namespace colored_json
