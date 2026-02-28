#include "query_engine.hpp"
#include <algorithm>
#include <set>

namespace colored_json {

// ============================================================================
// QueryEngine Implementation
// ============================================================================

QueryValue QueryEngine::execute(const QueryOp &op,
                                simdjson::ondemand::document &doc) {
  QueryValue input = QueryValue::fromSimdjson(doc);
  return executeOp(op, input);
}

QueryValue QueryEngine::execute(const QueryOp &op, const QueryValue &value) {
  return executeOp(op, value);
}

QueryValue QueryEngine::executeOp(const QueryOp &op, const QueryValue &input) {
  switch (op.type) {
  case QueryOpType::Identity:
    return input;

  case QueryOpType::Property:
    return evaluateProperty(input, op.prop_name);

  case QueryOpType::Index:
    if (op.isSlice) {
      return evaluateSlice(input, op.sliceStart, op.sliceEnd);
    }
    return evaluateIndex(input, op.index);

  case QueryOpType::Iterator:
    return evaluateIterator(input);

  case QueryOpType::Recursive:
    // For recursive, we need a subquery to apply
    if (!op.args.empty()) {
      return evaluateRecursive(input, *op.args[0]);
    }
    return input;

  case QueryOpType::Literal:
    return op.literalValue;

  case QueryOpType::Pipe: {
    if (op.args.size() != 2)
      return QueryValue::null();
    auto left = executeOp(*op.args[0], input);
    return executeOp(*op.args[1], left);
  }

  case QueryOpType::Variable: {
    auto it = vars_.find(op.prop_name);
    if (it != vars_.end()) {
      return it->second;
    }
    return QueryValue::null();
  }

  case QueryOpType::BindVariable: {
    if (op.args.size() != 2)
      return QueryValue::null();
    QueryValue bound_value = executeOp(*op.args[0], input);

    // Save previous value if exists (for shadowing)
    bool has_prev = vars_.count(op.prop_name) > 0;
    QueryValue prev_value;
    if (has_prev)
      prev_value = vars_[op.prop_name];

    vars_[op.prop_name] = bound_value;
    QueryValue result = executeOp(
        *op.args[1], input); // input remains the same for the rest of pipeline

    // Restore previous variable state
    if (has_prev) {
      vars_[op.prop_name] = prev_value;
    } else {
      vars_.erase(op.prop_name);
    }

    return result;
  }

  case QueryOpType::Assign: {
    if (op.args.size() != 2)
      return input;
    return applyAssignment(*op.args[0], input, input, *op.args[1], false);
  }

  case QueryOpType::UpdateAssign: {
    if (op.args.size() != 2)
      return input;
    return applyAssignment(*op.args[0], input, input, *op.args[1], true);
  }

  case QueryOpType::Select:
    return evaluateSelect(input, *op.args[0]);

  case QueryOpType::Map:
    return evaluateMap(input, *op.args[0]);

  case QueryOpType::Keys:
    return evaluateKeys(input);

  case QueryOpType::Values:
    return evaluateValues(input);

  case QueryOpType::Length:
    return evaluateLength(input);

  case QueryOpType::Sort:
    return evaluateSort(input);

  case QueryOpType::Unique:
    return evaluateUnique(input);

  case QueryOpType::Reverse:
    return evaluateReverse(input);

  case QueryOpType::Contains:
    if (op.args.size() >= 1) {
      QueryValue needle = executeOp(*op.args[0], QueryValue::null());
      return evaluateContains(input, needle);
    }
    return QueryValue(false);

  case QueryOpType::Has:
    if (op.args.size() >= 1 && op.args[0]->type == QueryOpType::Literal) {
      return evaluateHas(input, op.args[0]->literalValue.asString());
    }
    return QueryValue(false);

  case QueryOpType::Equal:
  case QueryOpType::NotEqual:
  case QueryOpType::Less:
  case QueryOpType::LessEqual:
  case QueryOpType::Greater:
  case QueryOpType::GreaterEqual:
  case QueryOpType::Add:
  case QueryOpType::Subtract:
  case QueryOpType::Multiply:
  case QueryOpType::Divide:
  case QueryOpType::Modulo:
  case QueryOpType::And:
  case QueryOpType::Or: {
    if (op.args.size() != 2)
      return QueryValue::null();
    QueryValue left = executeOp(*op.args[0], input);
    QueryValue right = executeOp(*op.args[1], input);
    return evaluateBinaryOp(op.type, left, right);
  }

  case QueryOpType::Not: {
    if (op.args.size() != 1)
      return QueryValue(false);
    QueryValue arg = executeOp(*op.args[0], input);
    return evaluateUnaryOp(op.type, arg);
  }

  case QueryOpType::Optional: {
    if (op.args.size() != 1)
      return QueryValue::null();
    try {
      return executeOp(*op.args[0], input);
    } catch (...) {
      // In a more strict engine, this would catch type errors.
      // For now, it evaluates and swallows C++ exceptions.
      return QueryValue::null();
    }
  }

  case QueryOpType::TryCatch: {
    if (op.args.size() != 2)
      return QueryValue::null();
    try {
      return executeOp(*op.args[0], input);
    } catch (...) {
      return executeOp(*op.args[1], input);
    }
  }

  case QueryOpType::Test: {
    if (op.args.size() != 1 || !input.isString() ||
        op.args[0]->type != QueryOpType::Literal ||
        !op.args[0]->literalValue.isString()) {
      return QueryValue(false);
    }
    const std::string &str = input.asString();
    const std::string &pattern = op.args[0]->literalValue.asString();
    try {
      std::regex re(pattern);
      return QueryValue(std::regex_search(str, re));
    } catch (...) {
      return QueryValue(false);
    }
  }

  case QueryOpType::Match: {
    if (op.args.size() != 1 || !input.isString() ||
        op.args[0]->type != QueryOpType::Literal ||
        !op.args[0]->literalValue.isString()) {
      return QueryValue::null();
    }
    const std::string &str = input.asString();
    const std::string &pattern = op.args[0]->literalValue.asString();
    try {
      std::regex re(pattern);
      std::smatch m;
      if (std::regex_search(str, m, re)) {
        QueryValue::ObjectType obj;
        obj.push_back({"string", QueryValue(m.str())});
        obj.push_back(
            {"offset", QueryValue(static_cast<double>(m.position()))});
        obj.push_back({"length", QueryValue(static_cast<double>(m.length()))});
        return QueryValue::object(std::move(obj));
      }
    } catch (...) {
    }
    return QueryValue::null();
  }

  case QueryOpType::Sub: {
    if (op.args.size() != 2 || !input.isString() ||
        op.args[0]->type != QueryOpType::Literal ||
        !op.args[0]->literalValue.isString() ||
        op.args[1]->type != QueryOpType::Literal ||
        !op.args[1]->literalValue.isString()) {
      return input;
    }
    const std::string &str = input.asString();
    const std::string &pattern = op.args[0]->literalValue.asString();
    const std::string &replacement = op.args[1]->literalValue.asString();
    try {
      std::regex re(pattern);
      return QueryValue(std::regex_replace(str, re, replacement));
    } catch (...) {
    }
    return input;
  }

  case QueryOpType::StringInterpolation: {
    std::string result = "";
    for (size_t i = 0; i < op.args.size(); ++i) {
      QueryValue val = executeOp(*op.args[i], input);
      if (val.isString()) {
        result += val.asString();
      } else if (!val.isNull()) {
        // use compact formatting for strings? toJson is usually fine.
        result += val.toJson();
      }
    }
    return QueryValue(result);
  }

  case QueryOpType::ObjectConstruction: {
    QueryValue::ObjectType obj;
    for (size_t i = 0; i < op.args.size(); ++i) {
      QueryValue val = executeOp(*op.args[i], input);
      if (!val.isNull()) {
        obj.push_back({op.object_keys[i], val});
      }
    }
    return QueryValue::object(std::move(obj));
  }

  case QueryOpType::ArrayConstruction: {
    QueryValue::ArrayType arr;
    for (size_t i = 0; i < op.args.size(); ++i) {
      QueryValue val = executeOp(*op.args[i], input);
      if (!val.isNull()) {
        arr.push_back(val);
      }
    }
    return QueryValue::array(std::move(arr));
  }

  default:
    return QueryValue::null();
  }
}

QueryValue QueryEngine::evaluateProperty(const QueryValue &input,
                                         const std::string &prop) {
  if (input.isObject()) {
    for (const auto &[key, val] : input.asObject()) {
      if (key == prop) {
        return val;
      }
    }
  } else if (input.isArray()) {
    // Property access on array: map over elements
    QueryValue::ArrayType result;
    for (const auto &elem : input.asArray()) {
      auto val = evaluateProperty(elem, prop);
      if (!val.isNull()) {
        result.push_back(val);
      }
    }
    if (!result.empty()) {
      return QueryValue::array(std::move(result));
    }
  }
  return QueryValue::null();
}

QueryValue QueryEngine::evaluateIndex(const QueryValue &input, int64_t index) {
  if (input.isArray()) {
    const auto &arr = input.asArray();
    int64_t actualIndex = index;
    if (actualIndex < 0) {
      actualIndex = static_cast<int64_t>(arr.size()) + actualIndex;
    }
    if (actualIndex >= 0 && actualIndex < static_cast<int64_t>(arr.size())) {
      return arr[static_cast<size_t>(actualIndex)];
    }
  } else if (input.isObject()) {
    // String index on object
    return evaluateProperty(input, std::to_string(index));
  }
  return QueryValue::null();
}

QueryValue QueryEngine::evaluateSlice(const QueryValue &input, int64_t start,
                                      int64_t end) {
  if (!input.isArray())
    return QueryValue::array({});

  const auto &arr = input.asArray();
  int64_t len = static_cast<int64_t>(arr.size());

  // Handle negative indices
  int64_t actualStart = start;
  if (actualStart < 0)
    actualStart = len + actualStart;
  if (actualStart < 0)
    actualStart = 0;
  if (actualStart > len)
    actualStart = len;

  int64_t actualEnd = end;
  if (actualEnd < 0)
    actualEnd = len + actualEnd;
  if (actualEnd < 0)
    actualEnd = 0;
  if (actualEnd > len)
    actualEnd = len;

  QueryValue::ArrayType result;
  for (int64_t i = actualStart; i < actualEnd; ++i) {
    result.push_back(arr[static_cast<size_t>(i)]);
  }
  return QueryValue::array(std::move(result));
}

QueryValue QueryEngine::evaluateIterator(const QueryValue &input) {
  if (input.isArray()) {
    return input; // Return array elements as-is for iteration context
  } else if (input.isObject()) {
    // Iterator on object returns values
    QueryValue::ArrayType result;
    for (const auto &[_, val] : input.asObject()) {
      result.push_back(val);
    }
    return QueryValue::array(std::move(result));
  }
  return QueryValue::array({});
}

QueryValue QueryEngine::evaluateRecursive(const QueryValue &input,
                                          const QueryOp &subquery) {
  QueryValue::ArrayType result;

  std::function<void(const QueryValue &)> collect = [&](const QueryValue &val) {
    // Try to match this value
    QueryValue matched = executeOp(subquery, val);
    if (!matched.isNull()) {
      if (matched.isArray()) {
        for (const auto &elem : matched.asArray()) {
          result.push_back(elem);
        }
      } else {
        result.push_back(matched);
      }
    }

    // Recurse into children
    if (val.isArray()) {
      for (const auto &elem : val.asArray()) {
        collect(elem);
      }
    } else if (val.isObject()) {
      for (const auto &[_, elem] : val.asObject()) {
        collect(elem);
      }
    }
  };

  collect(input);
  return QueryValue::array(std::move(result));
}

QueryValue QueryEngine::evaluateSelect(const QueryValue &input,
                                       const QueryOp &predicate) {
  if (!input.isArray()) {
    // If input is not array, just test the predicate
    QueryValue test = executeOp(predicate, input);
    if (test.isBool() && test.asBool()) {
      return input;
    }
    return QueryValue::null();
  }

  QueryValue::ArrayType result;
  for (const auto &elem : input.asArray()) {
    QueryValue test = executeOp(predicate, elem);
    if (test.isBool() && test.asBool()) {
      result.push_back(elem);
    }
  }
  return QueryValue::array(std::move(result));
}

QueryValue QueryEngine::evaluateMap(const QueryValue &input,
                                    const QueryOp &expr) {
  if (!input.isArray()) {
    return QueryValue::array({});
  }

  QueryValue::ArrayType result;
  for (const auto &elem : input.asArray()) {
    result.push_back(executeOp(expr, elem));
  }
  return QueryValue::array(std::move(result));
}

QueryValue QueryEngine::evaluateKeys(const QueryValue &input) {
  QueryValue::ArrayType result;
  if (input.isObject()) {
    for (const auto &[key, _] : input.asObject()) {
      result.push_back(QueryValue(key));
    }
  } else if (input.isArray()) {
    // Keys of array are indices
    for (size_t i = 0; i < input.asArray().size(); ++i) {
      result.push_back(QueryValue(static_cast<double>(i)));
    }
  }
  return QueryValue::array(std::move(result));
}

QueryValue QueryEngine::evaluateValues(const QueryValue &input) {
  return evaluateIterator(input);
}

QueryValue QueryEngine::evaluateLength(const QueryValue &input) {
  if (input.isArray()) {
    return QueryValue(static_cast<double>(input.asArray().size()));
  } else if (input.isObject()) {
    return QueryValue(static_cast<double>(input.asObject().size()));
  } else if (input.isString()) {
    return QueryValue(static_cast<double>(input.asString().size()));
  }
  return QueryValue(0.0);
}

QueryValue QueryEngine::evaluateSort(const QueryValue &input) {
  if (!input.isArray())
    return input;

  auto arr = input.asArray();
  std::sort(arr.begin(), arr.end());
  return QueryValue::array(std::move(arr));
}

QueryValue QueryEngine::evaluateUnique(const QueryValue &input) {
  if (!input.isArray())
    return input;

  std::vector<QueryValue> result;
  std::set<QueryValue> seen;
  for (const auto &elem : input.asArray()) {
    if (seen.insert(elem).second) {
      result.push_back(elem);
    }
  }
  return QueryValue::array(std::move(result));
}

QueryValue QueryEngine::evaluateReverse(const QueryValue &input) {
  if (!input.isArray())
    return input;

  auto arr = input.asArray();
  std::reverse(arr.begin(), arr.end());
  return QueryValue::array(std::move(arr));
}

QueryValue QueryEngine::evaluateContains(const QueryValue &input,
                                         const QueryValue &needle) {
  if (input.isArray()) {
    for (const auto &elem : input.asArray()) {
      if (elem == needle)
        return QueryValue(true);
    }
  } else if (input.isString() && needle.isString()) {
    return QueryValue(input.asString().find(needle.asString()) !=
                      std::string::npos);
  }
  return QueryValue(false);
}

QueryValue QueryEngine::evaluateHas(const QueryValue &input,
                                    const std::string &key) {
  if (input.isObject()) {
    for (const auto &[k, _] : input.asObject()) {
      if (k == key)
        return QueryValue(true);
    }
  }
  return QueryValue(false);
}

QueryValue QueryEngine::evaluateBinaryOp(QueryOpType type,
                                         const QueryValue &left,
                                         const QueryValue &right) {
  switch (type) {
  case QueryOpType::Equal:
    return QueryValue(left == right);
  case QueryOpType::NotEqual:
    return QueryValue(left != right);
  case QueryOpType::Less:
    return QueryValue(left < right);
  case QueryOpType::LessEqual:
    return QueryValue(left < right || left == right);
  case QueryOpType::Greater:
    return QueryValue(right < left);
  case QueryOpType::GreaterEqual:
    return QueryValue(right < left || left == right);
  case QueryOpType::Add:
    if (left.isNumber() && right.isNumber())
      return QueryValue(left.asNumber() + right.asNumber());
    if (left.isString() && right.isString())
      return QueryValue(left.asString() + right.asString());
    break;
  case QueryOpType::Subtract:
    if (left.isNumber() && right.isNumber())
      return QueryValue(left.asNumber() - right.asNumber());
    break;
  case QueryOpType::Multiply:
    if (left.isNumber() && right.isNumber())
      return QueryValue(left.asNumber() * right.asNumber());
    break;
  case QueryOpType::Divide:
    if (left.isNumber() && right.isNumber() && right.asNumber() != 0)
      return QueryValue(left.asNumber() / right.asNumber());
    break;
  case QueryOpType::Modulo:
    if (left.isNumber() && right.isNumber() && right.asNumber() != 0)
      return QueryValue(fmod(left.asNumber(), right.asNumber()));
    break;
  case QueryOpType::And:
    if (left.isBool() && right.isBool())
      return QueryValue(left.asBool() && right.asBool());
    break;
  case QueryOpType::Or:
    if (left.isBool() && right.isBool())
      return QueryValue(left.asBool() || right.asBool());
    break;
  default:
    break;
  }
  return QueryValue::null();
}

QueryValue QueryEngine::evaluateUnaryOp(QueryOpType type,
                                        const QueryValue &arg) {
  switch (type) {
  case QueryOpType::Not:
    if (arg.isBool())
      return QueryValue(!arg.asBool());
    // Truthiness: null, false, 0, empty array, empty string are falsy
    if (arg.isNull())
      return QueryValue(true);
    if (arg.isNumber())
      return QueryValue(arg.asNumber() == 0);
    if (arg.isString())
      return QueryValue(arg.asString().empty());
    if (arg.isArray())
      return QueryValue(arg.asArray().empty());
    return QueryValue(false);
  default:
    break;
  }
  return QueryValue::null();
}

QueryValue QueryEngine::applyAssignment(const QueryOp &target,
                                        const QueryValue &input,
                                        const QueryValue &rootInput,
                                        const QueryOp &rhs, bool isUpdate) {
  if (target.type == QueryOpType::Property) {
    if (!input.isObject())
      return input;
    auto obj = input.asObject();
    bool found = false;
    for (auto &[k, v] : obj) {
      if (k == target.prop_name) {
        if (isUpdate) {
          v = executeOp(rhs, v);
        } else {
          v = executeOp(rhs, rootInput);
        }
        found = true;
        break;
      }
    }
    if (!found) {
      QueryValue newVal = isUpdate ? executeOp(rhs, QueryValue::null())
                                   : executeOp(rhs, rootInput);
      obj.push_back({target.prop_name, newVal});
    }
    return QueryValue::object(std::move(obj));
  }

  if (target.type == QueryOpType::Index && !target.isSlice) {
    if (!input.isArray())
      return input;
    auto arr = input.asArray();
    if (target.index >= 0 && target.index < static_cast<int64_t>(arr.size())) {
      if (isUpdate) {
        arr[target.index] = executeOp(rhs, arr[target.index]);
      } else {
        arr[target.index] = executeOp(rhs, rootInput);
      }
    }
    return QueryValue::array(std::move(arr));
  }

  if (target.type == QueryOpType::Pipe) {
    if (target.args.size() != 2)
      return input;
    QueryValue lhs_val = executeOp(*target.args[0], input);
    QueryValue new_lhs_val =
        applyAssignment(*target.args[1], lhs_val, rootInput, rhs, isUpdate);
    auto mock_rhs = QueryOp::literal(new_lhs_val);
    return applyAssignment(*target.args[0], input, rootInput, *mock_rhs, false);
  }

  return input;
}

} // namespace colored_json
