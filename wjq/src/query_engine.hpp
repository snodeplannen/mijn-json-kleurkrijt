#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include "json_parser.hpp"

namespace colored_json {

// Forward declarations
class QueryValue;

// Types of query values
enum class QueryValueType { Null, Bool, Number, String, Array, Object };

// Represents a value in the query engine
class QueryValue {
public:
  using ObjectType = std::vector<std::pair<std::string, QueryValue>>;
  using ArrayType = std::vector<QueryValue>;

private:
  struct NullValue {};
  std::variant<NullValue, bool, double, std::string, ArrayType, ObjectType>
      data_;

public:
  QueryValue() : data_(NullValue{}) {}
  QueryValue(bool b) : data_(b) {}
  QueryValue(double d) : data_(d) {}
  QueryValue(const std::string &s) : data_(s) {}
  QueryValue(std::string_view s) : data_(std::string(s)) {}
  QueryValue(ArrayType arr) : data_(std::move(arr)) {}
  QueryValue(ObjectType obj) : data_(std::move(obj)) {}

  static QueryValue null() { return QueryValue{}; }
  static QueryValue array(ArrayType arr) { return QueryValue{std::move(arr)}; }
  static QueryValue object(ObjectType obj) {
    return QueryValue{std::move(obj)};
  }

  QueryValueType type() const {
    return std::visit(
        [](const auto &val) -> QueryValueType {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, NullValue>)
            return QueryValueType::Null;
          else if constexpr (std::is_same_v<T, bool>)
            return QueryValueType::Bool;
          else if constexpr (std::is_same_v<T, double>)
            return QueryValueType::Number;
          else if constexpr (std::is_same_v<T, std::string>)
            return QueryValueType::String;
          else if constexpr (std::is_same_v<T, ArrayType>)
            return QueryValueType::Array;
          else if constexpr (std::is_same_v<T, ObjectType>)
            return QueryValueType::Object;
          return QueryValueType::Null;
        },
        data_);
  }

  bool isNull() const { return std::holds_alternative<NullValue>(data_); }
  bool isBool() const { return std::holds_alternative<bool>(data_); }
  bool isNumber() const { return std::holds_alternative<double>(data_); }
  bool isString() const { return std::holds_alternative<std::string>(data_); }
  bool isArray() const { return std::holds_alternative<ArrayType>(data_); }
  bool isObject() const { return std::holds_alternative<ObjectType>(data_); }

  bool asBool() const { return std::get<bool>(data_); }
  double asNumber() const { return std::get<double>(data_); }
  const std::string &asString() const { return std::get<std::string>(data_); }
  const ArrayType &asArray() const { return std::get<ArrayType>(data_); }
  const ObjectType &asObject() const { return std::get<ObjectType>(data_); }

  ArrayType &asArray() { return std::get<ArrayType>(data_); }
  ObjectType &asObject() { return std::get<ObjectType>(data_); }

  // Convert from simdjson value
  static QueryValue fromSimdjson(simdjson::ondemand::value val);
  static QueryValue fromSimdjson(simdjson::ondemand::document &doc);

  // Convert to JSON string
  std::string toJson() const;
  std::string toJson(int indent, int currentIndent = 0) const;

  // Comparison operators
  bool operator==(const QueryValue &other) const;
  bool operator!=(const QueryValue &other) const { return !(*this == other); }
  bool operator<(const QueryValue &other) const;

  // Get size (for arrays and objects)
  size_t size() const {
    if (isArray())
      return asArray().size();
    if (isObject())
      return asObject().size();
    return 0;
  }

  // Get keys (for objects)
  std::vector<std::string> keys() const {
    if (!isObject())
      return {};
    std::vector<std::string> result;
    for (const auto &[key, _] : asObject()) {
      result.push_back(key);
    }
    return result;
  }

  // Index access (for arrays)
  const QueryValue &operator[](size_t index) const {
    if (isArray())
      return asArray()[index];
    static QueryValue null_val;
    return null_val;
  }

  // Key access (for objects)
  QueryValue operator[](const std::string &key) const {
    if (isObject()) {
      for (const auto &[k, v] : asObject()) {
        if (k == key)
          return v;
      }
    }
    return QueryValue::null();
  }
};

// Query operator types
enum class QueryOpType {
  Identity,            // .
  Property,            // .foo, .["foo"]
  Index,               // [0], [1:3]
  Iterator,            // .[]
  Recursive,           // ..
  Pipe,                // |
  Select,              // select(predicate)
  Map,                 // map(expr)
  Keys,                // keys
  Values,              // values
  Length,              // length
  Sort,                // sort
  Unique,              // unique
  Reverse,             // reverse
  Contains,            // contains(x)
  Has,                 // has(key)
  Test,                // test("regex")
  Match,               // match("regex")
  Sub,                 // sub("regex", "replacement")
  Optional,            // expr?
  TryCatch,            // try expr catch expr
  Variable,            // $var
  BindVariable,        // expr as $var | next
  Add,                 // +
  Subtract,            // -
  Multiply,            // *
  Divide,              // /
  Modulo,              // %
  Equal,               // ==
  NotEqual,            // !=
  Less,                // <
  LessEqual,           // <=
  Greater,             // >
  GreaterEqual,        // >=
  And,                 // and
  Or,                  // or
  Not,                 // not
  Literal,             // literal value
  ObjectConstruction,  // { key: expr, ... }
  ArrayConstruction,   // [ expr, ... ]
  StringInterpolation, // "literal \(expr) literal"
  Assign,              // =
  UpdateAssign         // |=
};

// Query operator node
struct QueryOp {
  QueryOpType type;
  std::string prop_name;                // For Property
  int64_t index = 0;                    // For Index
  int64_t sliceStart = 0, sliceEnd = 0; // For slice [start:end]
  bool isSlice = false;                 // Whether this is a slice
  QueryValue literalValue;              // For Literal
  std::vector<std::unique_ptr<QueryOp>>
      args;                             // For function arguments, binary ops
  std::vector<std::string> object_keys; // For ObjectConstruction keys

  QueryOp(QueryOpType t) : type(t) {}

  // Factory methods
  static std::unique_ptr<QueryOp> identity() {
    return std::make_unique<QueryOp>(QueryOpType::Identity);
  }
  static std::unique_ptr<QueryOp> make_property(const std::string &prop) {
    auto op = std::make_unique<QueryOp>(QueryOpType::Property);
    op->prop_name = prop;
    return op;
  }
  static std::unique_ptr<QueryOp> make_index(int64_t idx) {
    auto op = std::make_unique<QueryOp>(QueryOpType::Index);
    op->index = idx;
    return op;
  }
  static std::unique_ptr<QueryOp> make_slice(int64_t start, int64_t end) {
    auto op = std::make_unique<QueryOp>(QueryOpType::Index);
    op->isSlice = true;
    op->sliceStart = start;
    op->sliceEnd = end;
    return op;
  }
  static std::unique_ptr<QueryOp> iterator() {
    return std::make_unique<QueryOp>(QueryOpType::Iterator);
  }
  static std::unique_ptr<QueryOp> recursive() {
    return std::make_unique<QueryOp>(QueryOpType::Recursive);
  }
  static std::unique_ptr<QueryOp> literal(const QueryValue &val) {
    auto op = std::make_unique<QueryOp>(QueryOpType::Literal);
    op->literalValue = val;
    return op;
  }
  static std::unique_ptr<QueryOp> pipe(std::unique_ptr<QueryOp> left,
                                       std::unique_ptr<QueryOp> right) {
    auto op = std::make_unique<QueryOp>(QueryOpType::Pipe);
    op->args.push_back(std::move(left));
    op->args.push_back(std::move(right));
    return op;
  }
  static std::unique_ptr<QueryOp>
  function(QueryOpType type, std::vector<std::unique_ptr<QueryOp>> args) {
    auto op = std::make_unique<QueryOp>(type);
    op->args = std::move(args);
    return op;
  }
  static std::unique_ptr<QueryOp> binary(QueryOpType type,
                                         std::unique_ptr<QueryOp> left,
                                         std::unique_ptr<QueryOp> right) {
    auto op = std::make_unique<QueryOp>(type);
    op->args.push_back(std::move(left));
    op->args.push_back(std::move(right));
    return op;
  }
  static std::unique_ptr<QueryOp> unary(QueryOpType type,
                                        std::unique_ptr<QueryOp> arg) {
    auto op = std::make_unique<QueryOp>(type);
    op->args.push_back(std::move(arg));
    return op;
  }
  static std::unique_ptr<QueryOp>
  make_object(std::vector<std::string> keys,
              std::vector<std::unique_ptr<QueryOp>> vals) {
    auto op = std::make_unique<QueryOp>(QueryOpType::ObjectConstruction);
    op->object_keys = std::move(keys);
    op->args = std::move(vals);
    return op;
  }
  static std::unique_ptr<QueryOp>
  make_array(std::vector<std::unique_ptr<QueryOp>> elements) {
    auto op = std::make_unique<QueryOp>(QueryOpType::ArrayConstruction);
    op->args = std::move(elements);
    return op;
  }
  static std::unique_ptr<QueryOp>
  make_string_interpolation(std::vector<std::unique_ptr<QueryOp>> parts) {
    auto op = std::make_unique<QueryOp>(QueryOpType::StringInterpolation);
    op->args = std::move(parts);
    return op;
  }
};

// Query parser - forward declaration only, implementation in cpp
class JqQueryParser;

// Factory function to create parser
std::unique_ptr<JqQueryParser> createQueryParser(const std::string &query_str);

// Query execution engine
class QueryEngine {
  std::map<std::string, QueryValue> vars_;

public:
  QueryEngine() = default;
  QueryEngine(std::map<std::string, QueryValue> vars)
      : vars_(std::move(vars)) {}

  static std::unique_ptr<QueryOp> parse(const std::string &query,
                                        std::string &error);
  QueryValue execute(const QueryOp &op, simdjson::ondemand::document &doc);
  QueryValue execute(const QueryOp &op, const QueryValue &value);

private:
  QueryValue executeOp(const QueryOp &op, const QueryValue &input);
  QueryValue applyAssignment(const QueryOp &target, const QueryValue &input,
                             const QueryValue &rootInput, const QueryOp &rhs,
                             bool isUpdate);
  QueryValue evaluateProperty(const QueryValue &input, const std::string &prop);
  QueryValue evaluateIndex(const QueryValue &input, int64_t index);
  QueryValue evaluateSlice(const QueryValue &input, int64_t start, int64_t end);
  QueryValue evaluateIterator(const QueryValue &input);
  QueryValue evaluateRecursive(const QueryValue &input,
                               const QueryOp &subquery);
  QueryValue evaluateSelect(const QueryValue &input, const QueryOp &predicate);
  QueryValue evaluateMap(const QueryValue &input, const QueryOp &expr);
  QueryValue evaluateKeys(const QueryValue &input);
  QueryValue evaluateValues(const QueryValue &input);
  QueryValue evaluateLength(const QueryValue &input);
  QueryValue evaluateSort(const QueryValue &input);
  QueryValue evaluateUnique(const QueryValue &input);
  QueryValue evaluateReverse(const QueryValue &input);
  QueryValue evaluateContains(const QueryValue &input,
                              const QueryValue &needle);
  QueryValue evaluateHas(const QueryValue &input, const std::string &key);

  // Arithmetic and comparison
  QueryValue evaluateBinaryOp(QueryOpType type, const QueryValue &left,
                              const QueryValue &right);
  QueryValue evaluateUnaryOp(QueryOpType type, const QueryValue &arg);
};

// Utility functions
QueryValue parseJson(const std::string &json);
std::string escapeJsonString(const std::string &s);

} // namespace colored_json
