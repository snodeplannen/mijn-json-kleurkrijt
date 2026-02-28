#pragma once
#include <regex>
#include <string>
#include <vector>

namespace colored_json {

// Forward declarations
struct d;
struct StyleContext;

// Match result containing match info
struct MatchResult {
  bool matched = false;
  double match_strength = 1.0; // For partial matches or priority
  std::string matched_pattern;

  explicit MatchResult(bool m = false) : matched(m) {}
  MatchResult(bool m, double strength, std::string pattern = "")
      : matched(m), match_strength(strength),
        matched_pattern(std::move(pattern)) {}
};

// Numeric comparison types
enum class CompareOp {
  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual
};

// Base matcher interface
class Matcher {
public:
  virtual ~Matcher() = default;
  virtual MatchResult match(const std::string &value,
                            const StyleContext &ctx) const = 0;
  virtual std::string describe() const = 0;
  virtual std::unique_ptr<Matcher> clone() const = 0;
};

// String keyword matcher
class KeywordMatcher : public Matcher {
  std::vector<std::string> keywords;
  bool case_sensitive;

public:
  explicit KeywordMatcher(std::vector<std::string> kws,
                          bool case_sensitive = true)
      : keywords(std::move(kws)), case_sensitive(case_sensitive) {}

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override {
    return std::make_unique<KeywordMatcher>(*this);
  }
};

// Regex matcher
class RegexMatcher : public Matcher {
  std::regex pattern;
  std::string pattern_str;

public:
  explicit RegexMatcher(const std::string &pattern_str);

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override {
    return std::make_unique<RegexMatcher>(pattern_str);
  }
};

// Numeric range matcher
class RangeMatcher : public Matcher {
  double min_val;
  double max_val;
  bool inclusive_min;
  bool inclusive_max;

public:
  RangeMatcher(double min_v, double max_v, bool incl_min = true,
               bool incl_max = true)
      : min_val(min_v), max_val(max_v), inclusive_min(incl_min),
        inclusive_max(incl_max) {}

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override {
    return std::make_unique<RangeMatcher>(*this);
  }
};

// Numeric comparison matcher
class CompareMatcher : public Matcher {
  double compare_val;
  CompareOp op;

public:
  CompareMatcher(double val, CompareOp operation)
      : compare_val(val), op(operation) {}

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override {
    return std::make_unique<CompareMatcher>(*this);
  }
};

// Path matcher (matches JSON path patterns like "user.*.name")
class PathMatcher : public Matcher {
  std::string pattern;
  std::regex path_regex;

public:
  explicit PathMatcher(std::string path_pattern);

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override {
    return std::make_unique<PathMatcher>(pattern);
  }
};

// Composite matchers
class AnyMatcher : public Matcher {
  std::vector<std::unique_ptr<Matcher>> matchers;

public:
  explicit AnyMatcher(std::vector<std::unique_ptr<Matcher>> m)
      : matchers(std::move(m)) {}

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override;
};

class AllMatcher : public Matcher {
  std::vector<std::unique_ptr<Matcher>> matchers;

public:
  explicit AllMatcher(std::vector<std::unique_ptr<Matcher>> m)
      : matchers(std::move(m)) {}

  MatchResult match(const std::string &value,
                    const StyleContext &ctx) const override;
  std::string describe() const override;
  std::unique_ptr<Matcher> clone() const override;
};

// Factory functions for easy matcher creation
inline std::unique_ptr<Matcher> keyword(std::vector<std::string> kws,
                                        bool case_sensitive = true) {
  return std::make_unique<KeywordMatcher>(std::move(kws), case_sensitive);
}

inline std::unique_ptr<Matcher> regex(const std::string &pattern) {
  return std::make_unique<RegexMatcher>(pattern);
}

inline std::unique_ptr<Matcher> range(double min, double max,
                                      bool inclusive_min = true,
                                      bool inclusive_max = true) {
  return std::make_unique<RangeMatcher>(min, max, inclusive_min, inclusive_max);
}

inline std::unique_ptr<Matcher> greater_than(double val) {
  return std::make_unique<CompareMatcher>(val, CompareOp::Greater);
}

inline std::unique_ptr<Matcher> less_than(double val) {
  return std::make_unique<CompareMatcher>(val, CompareOp::Less);
}

inline std::unique_ptr<Matcher> equal_to(double val) {
  return std::make_unique<CompareMatcher>(val, CompareOp::Equal);
}

inline std::unique_ptr<Matcher> path(const std::string &pattern) {
  return std::make_unique<PathMatcher>(pattern);
}

// Helper to combine matchers
inline std::unique_ptr<Matcher>
any_of(std::vector<std::unique_ptr<Matcher>> matchers) {
  return std::make_unique<AnyMatcher>(std::move(matchers));
}

inline std::unique_ptr<Matcher>
all_of(std::vector<std::unique_ptr<Matcher>> matchers) {
  return std::make_unique<AllMatcher>(std::move(matchers));
}

} // namespace colored_json
