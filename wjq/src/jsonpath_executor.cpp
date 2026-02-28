#include "jsonpath.hpp"

namespace colored_json {

// ============================================================================
// JsonPathEngine Implementation
// ============================================================================

QueryValue JsonPathEngine::execute(const JsonPathNode *path,
                                   const QueryValue &root) {
  if (!path)
    return QueryValue::null();

  auto results = evaluate_node(path, root);

  if (results.empty()) {
    return QueryValue::null();
  }
  if (results.size() == 1) {
    return results[0];
  }

  // Return as array
  QueryValue::ArrayType arr;
  for (auto &r : results) {
    arr.push_back(std::move(r));
  }
  return QueryValue::array(std::move(arr));
}

QueryValue JsonPathEngine::execute(const JsonPathNode *path,
                                   simdjson::ondemand::document &doc) {
  auto root = QueryValue::fromSimdjson(doc);
  return execute(path, root);
}

std::vector<QueryValue>
JsonPathEngine::evaluate_node(const JsonPathNode *node,
                              const QueryValue &context) {
  if (!node)
    return {context};

  std::vector<QueryValue> results;

  switch (node->type) {
  case JsonPathNodeType::Root: {
    // Root just passes through to next
    if (node->next) {
      return evaluate_node(node->next.get(), context);
    }
    return {context};
  }

  case JsonPathNodeType::Property: {
    if (context.isObject()) {
      const auto &obj = context.asObject();
      for (const auto &[key, value] : obj) {
        if (key == node->property_name) {
          // std::cerr << "DEBUG Property " << node->property_name << " found,
          // hasNext=" << (node->next ? 1 : 0) << "\n";
          if (node->next) {
            auto sub = evaluate_node(node->next.get(), value);
            results.insert(results.end(), sub.begin(), sub.end());
          } else {
            results.push_back(value);
          }
        }
      }
    }
    break;
  }

  case JsonPathNodeType::Index: {
    if (context.isArray()) {
      const auto &arr = context.asArray();
      int idx = node->index;
      if (idx < 0) {
        idx = static_cast<int>(arr.size()) + idx;
      }
      if (idx >= 0 && idx < static_cast<int>(arr.size())) {
        if (node->next) {
          return evaluate_node(node->next.get(), arr[idx]);
        } else {
          return {arr[idx]};
        }
      }
    }
    break;
  }

  case JsonPathNodeType::Slice: {
    if (context.isArray()) {
      const auto &arr = context.asArray();
      int start = node->slice_start;
      int end = node->slice_end;
      int step = node->slice_step;

      if (start < 0)
        start = static_cast<int>(arr.size()) + start;
      if (end < 0)
        end = static_cast<int>(arr.size()) + end;
      if (end < 0 || end > static_cast<int>(arr.size()))
        end = static_cast<int>(arr.size());

      if (step > 0) {
        for (int i = start; i < end; i += step) {
          if (i >= 0 && i < static_cast<int>(arr.size())) {
            if (node->next) {
              auto sub = evaluate_node(node->next.get(), arr[i]);
              results.insert(results.end(), sub.begin(), sub.end());
            } else {
              results.push_back(arr[i]);
            }
          }
        }
      } else if (step < 0) {
        for (int i = start; i > end; i += step) {
          if (i >= 0 && i < static_cast<int>(arr.size())) {
            if (node->next) {
              auto sub = evaluate_node(node->next.get(), arr[i]);
              results.insert(results.end(), sub.begin(), sub.end());
            } else {
              results.push_back(arr[i]);
            }
          }
        }
      }
    }
    break;
  }

  case JsonPathNodeType::Wildcard: {
    if (context.isObject()) {
      const auto &obj = context.asObject();
      for (const auto &[key, value] : obj) {
        if (node->next) {
          auto sub = evaluate_node(node->next.get(), value);
          results.insert(results.end(), sub.begin(), sub.end());
        } else {
          results.push_back(value);
        }
      }
    } else if (context.isArray()) {
      const auto &arr = context.asArray();
      for (const auto &item : arr) {
        if (node->next) {
          auto sub = evaluate_node(node->next.get(), item);
          results.insert(results.end(), sub.begin(), sub.end());
        } else {
          results.push_back(item);
        }
      }
    }
    break;
  }

  case JsonPathNodeType::RecursiveDescent: {
    // std::cerr << "DEBUG RecursiveDescent, next type=" << (node->next ?
    // (int)node->next->type : -1)
    //           << " next->next type=" << ((node->next && node->next->next) ?
    //           (int)node->next->next->type : -1) << "\n";
    auto recursive_results = evaluate_recursive(node->next.get(), context);
    results.insert(results.end(), recursive_results.begin(),
                   recursive_results.end());
    break;
  }

  case JsonPathNodeType::Union: {
    for (const auto &item : node->union_items) {
      if (std::holds_alternative<std::string>(item)) {
        const auto &name = std::get<std::string>(item);
        if (context.isObject()) {
          const auto &obj = context.asObject();
          for (const auto &[key, value] : obj) {
            if (key == name) {
              if (node->next) {
                auto sub = evaluate_node(node->next.get(), value);
                results.insert(results.end(), sub.begin(), sub.end());
              } else {
                results.push_back(value);
              }
            }
          }
        }
      } else {
        int idx = std::get<int>(item);
        if (context.isArray()) {
          const auto &arr = context.asArray();
          if (idx < 0)
            idx = static_cast<int>(arr.size()) + idx;
          if (idx >= 0 && idx < static_cast<int>(arr.size())) {
            if (node->next) {
              auto sub = evaluate_node(node->next.get(), arr[idx]);
              results.insert(results.end(), sub.begin(), sub.end());
            } else {
              results.push_back(arr[idx]);
            }
          }
        }
      }
    }
    break;
  }

  case JsonPathNodeType::Filter: {
    // std::cerr << "DEBUG Filter case, context isArray=" << context.isArray()
    // << "\n";
    if (context.isArray()) {
      const auto &arr = context.asArray();
      // std::cerr << "DEBUG Filter array size=" << arr.size() << "\n";
      for (const auto &item : arr) {
        if (evaluate_filter(node->filter_expr.get(), item, context)) {
          if (node->next) {
            auto sub = evaluate_node(node->next.get(), item);
            results.insert(results.end(), sub.begin(), sub.end());
          } else {
            results.push_back(item);
          }
        }
      }
    } else if (context.isObject()) {
      // Filter on object values
      const auto &obj = context.asObject();
      for (const auto &[key, value] : obj) {
        if (evaluate_filter(node->filter_expr.get(), value, context)) {
          if (node->next) {
            auto sub = evaluate_node(node->next.get(), value);
            results.insert(results.end(), sub.begin(), sub.end());
          } else {
            results.push_back(value);
          }
        }
      }
    }
    break;
  }
  }

  return results;
}

std::vector<QueryValue>
JsonPathEngine::evaluate_recursive(const JsonPathNode *next,
                                   const QueryValue &context) {
  std::vector<QueryValue> results;

  // First evaluate current node if it matches
  if (next) {
    auto current = evaluate_node(next, context);
    results.insert(results.end(), current.begin(), current.end());
  }

  // Then recurse into children
  if (context.isObject()) {
    const auto &obj = context.asObject();
    for (const auto &[key, value] : obj) {
      auto sub = evaluate_recursive(next, value);
      results.insert(results.end(), sub.begin(), sub.end());
    }
  } else if (context.isArray()) {
    const auto &arr = context.asArray();
    for (const auto &item : arr) {
      auto sub = evaluate_recursive(next, item);
      results.insert(results.end(), sub.begin(), sub.end());
    }
  }

  return results;
}

std::vector<QueryValue> JsonPathEngine::evaluate_recursive_with_filter(
    const JsonPathNode *target, const QueryValue &context,
    const JsonPathNode *filter_node, const QueryValue &root) {

  std::vector<QueryValue> results;

  // First check if current context matches target and passes filter
  if (target) {
    auto matches = evaluate_node(target, context);
    for (const auto &match : matches) {
      // Apply filter
      if (filter_node && filter_node->type == JsonPathNodeType::Filter) {
        if (match.isArray()) {
          // If match is an array, filter each element
          for (const auto &item : match.asArray()) {
            if (evaluate_filter(filter_node->filter_expr.get(), item, root)) {
              if (filter_node->next) {
                auto sub = evaluate_node(filter_node->next.get(), item);
                results.insert(results.end(), sub.begin(), sub.end());
              } else {
                results.push_back(item);
              }
            }
          }
        } else {
          // Single value, check filter directly
          if (evaluate_filter(filter_node->filter_expr.get(), match, root)) {
            if (filter_node->next) {
              auto sub = evaluate_node(filter_node->next.get(), match);
              results.insert(results.end(), sub.begin(), sub.end());
            } else {
              results.push_back(match);
            }
          }
        }
      } else {
        results.push_back(match);
      }
    }
  }

  // Recurse into children
  if (context.isObject()) {
    const auto &obj = context.asObject();
    for (const auto &[key, value] : obj) {
      auto sub =
          evaluate_recursive_with_filter(target, value, filter_node, root);
      results.insert(results.end(), sub.begin(), sub.end());
    }
  } else if (context.isArray()) {
    const auto &arr = context.asArray();
    for (const auto &item : arr) {
      auto sub =
          evaluate_recursive_with_filter(target, item, filter_node, root);
      results.insert(results.end(), sub.begin(), sub.end());
    }
  }

  return results;
}

bool JsonPathEngine::evaluate_filter(const FilterExpr *expr,
                                     const QueryValue &context,
                                     const QueryValue &root) {
  if (!expr)
    return true;

  // Debug: uncomment to see filter evaluations
  // std::cerr << "DEBUG evaluate_filter type=" << (int)expr->type << "\n";

  switch (expr->type) {
  case FilterExpr::Type::Comparison: {
    auto left_val = evaluate_filter_value(expr->left.get(), context, root);
    auto right_val = evaluate_filter_value(expr->right.get(), context, root);

    switch (expr->op) {
    case FilterOp::Eq:
      return left_val == right_val;
    case FilterOp::Ne:
      return !(left_val == right_val);
    case FilterOp::Lt: {
      if (left_val.isNumber() && right_val.isNumber()) {
        bool result = left_val.asNumber() < right_val.asNumber();
        // Debug: uncomment to see comparisons
        std::cerr << "DEBUG: " << left_val.asNumber() << " < "
                  << right_val.asNumber() << " = " << result << "\n";
        return result;
      }
      if (left_val.isString() && right_val.isString()) {
        return left_val.asString() < right_val.asString();
      }
      return false;
    }
    case FilterOp::Le: {
      if (left_val.isNumber() && right_val.isNumber()) {
        return left_val.asNumber() <= right_val.asNumber();
      }
      if (left_val.isString() && right_val.isString()) {
        return left_val.asString() <= right_val.asString();
      }
      return false;
    }
    case FilterOp::Gt: {
      if (left_val.isNumber() && right_val.isNumber()) {
        return left_val.asNumber() > right_val.asNumber();
      }
      if (left_val.isString() && right_val.isString()) {
        return left_val.asString() > right_val.asString();
      }
      return false;
    }
    case FilterOp::Ge: {
      if (left_val.isNumber() && right_val.isNumber()) {
        return left_val.asNumber() >= right_val.asNumber();
      }
      if (left_val.isString() && right_val.isString()) {
        return left_val.asString() >= right_val.asString();
      }
      return false;
    }
    default:
      return false;
    }
  }

  case FilterExpr::Type::Logical: {
    bool left = evaluate_filter(expr->left.get(), context, root);
    bool right = evaluate_filter(expr->right.get(), context, root);

    if (expr->op == FilterOp::And) {
      return left && right;
    } else if (expr->op == FilterOp::Or) {
      return left || right;
    }
    return false;
  }

  case FilterExpr::Type::Literal:
    // In boolean context, check truthiness
    if (expr->value.isBool()) {
      return expr->value.asBool();
    }
    if (expr->value.isNull()) {
      return false;
    }
    return true;

  case FilterExpr::Type::Path: {
    // Check if path exists and is truthy
    auto val = evaluate_filter_value(expr, context, root);
    if (val.isNull())
      return false;
    if (val.isBool())
      return val.asBool();
    if (val.isNumber())
      return val.asNumber() != 0;
    if (val.isString())
      return !val.asString().empty();
    return true; // arrays and objects are truthy
  }

  case FilterExpr::Type::Exists:
    return !evaluate_filter_value(expr, context, root).isNull();

  case FilterExpr::Type::Function: {
    auto val = evaluate_filter_value(expr, context, root);
    if (val.isNull())
      return false;
    if (val.isBool())
      return val.asBool();
    if (val.isNumber())
      return val.asNumber() != 0;
    return true;
  }
  }

  return false;
}

QueryValue JsonPathEngine::evaluate_filter_value(const FilterExpr *expr,
                                                 const QueryValue &context,
                                                 const QueryValue &root) {
  if (!expr)
    return QueryValue::null();

  switch (expr->type) {
  case FilterExpr::Type::Literal:
    return expr->value;

  case FilterExpr::Type::Path: {
    // Parse the path and evaluate
    const std::string &path_str = expr->path;

    if (path_str == "@") {
      return context;
    }

    if (path_str == "$") {
      return root;
    }

    // Handle @.property or $.property
    const QueryValue *current = nullptr;
    size_t pos = 0;

    if (path_str[0] == '@') {
      current = &context;
      pos = 1;
    } else if (path_str[0] == '$') {
      current = &root;
      pos = 1;
    }

    if (!current) {
      return QueryValue::null();
    }

    // Parse rest of path
    while (pos < path_str.size() && current) {
      if (path_str[pos] == '.') {
        pos++;
        std::string prop;
        while (pos < path_str.size() &&
               (std::isalnum(path_str[pos]) || path_str[pos] == '_')) {
          prop += path_str[pos++];
        }

        if (current->isObject()) {
          const auto &obj = current->asObject();
          bool found = false;
          for (const auto &[key, val] : obj) {
            if (key == prop) {
              current = &val;
              found = true;
              break;
            }
          }
          if (!found)
            return QueryValue::null();
        } else {
          return QueryValue::null();
        }
      } else if (path_str[pos] == '[') {
        pos++;
        if (pos < path_str.size() && path_str[pos] == '\'') {
          // ['property']
          std::string prop;
          pos++;
          while (pos < path_str.size() && path_str[pos] != '\'') {
            prop += path_str[pos++];
          }
          if (pos < path_str.size())
            pos++; // skip '\''
          if (pos < path_str.size())
            pos++; // skip ']'

          if (current->isObject()) {
            const auto &obj = current->asObject();
            bool found = false;
            for (const auto &[key, val] : obj) {
              if (key == prop) {
                current = &val;
                found = true;
                break;
              }
            }
            if (!found)
              return QueryValue::null();
          } else {
            return QueryValue::null();
          }
        } else {
          // [index]
          int idx = 0;
          bool negative = false;
          if (path_str[pos] == '-') {
            negative = true;
            pos++;
          }
          while (pos < path_str.size() && std::isdigit(path_str[pos])) {
            idx = idx * 10 + (path_str[pos] - '0');
            ;
            pos++;
          }
          if (negative)
            idx = -idx;
          if (pos < path_str.size())
            pos++; // skip ']'

          if (current->isArray()) {
            const auto &arr = current->asArray();
            if (idx < 0)
              idx = static_cast<int>(arr.size()) + idx;
            if (idx >= 0 && idx < static_cast<int>(arr.size())) {
              current = &arr[idx];
            } else {
              return QueryValue::null();
            }
          } else {
            return QueryValue::null();
          }
        }
      } else {
        break;
      }
    }

    return *current;
  }

  case FilterExpr::Type::Function: {
    if (expr->func_name == "length") {
      if (expr->args.empty()) {
        // length of context
        if (context.isArray()) {
          return QueryValue(static_cast<double>(context.asArray().size()));
        } else if (context.isObject()) {
          return QueryValue(static_cast<double>(context.asObject().size()));
        } else if (context.isString()) {
          return QueryValue(static_cast<double>(context.asString().size()));
        }
      } else {
        auto arg_val =
            evaluate_filter_value(expr->args[0].get(), context, root);
        if (arg_val.isArray()) {
          return QueryValue(static_cast<double>(arg_val.asArray().size()));
        } else if (arg_val.isObject()) {
          return QueryValue(static_cast<double>(arg_val.asObject().size()));
        } else if (arg_val.isString()) {
          return QueryValue(static_cast<double>(arg_val.asString().size()));
        }
      }
    } else if (expr->func_name == "count") {
      if (expr->args.empty()) {
        return QueryValue(static_cast<double>(0));
      }
      auto arg_val = evaluate_filter_value(expr->args[0].get(), context, root);
      if (arg_val.isArray()) {
        return QueryValue(static_cast<double>(arg_val.asArray().size()));
      }
    }
    return QueryValue::null();
  }

  default:
    return QueryValue::null();
  }
}

// ============================================================================
// JsonQuery Unified Interface
// ============================================================================

std::string JsonQuery::last_error_;

std::string JsonQuery::execute(const std::string &json,
                               const std::string &query,
                               const std::map<std::string, QueryValue> &vars) {
  last_error_.clear();

  try {
    // Parse JSON
    simdjson::padded_string padded(json);
    simdjson::ondemand::parser parser;
    auto doc_result = parser.iterate(padded);
    auto &doc = doc_result.value();

    QueryValue result;

    if (is_jsonpath(query)) {
      // Parse and execute JSONPath
      JsonPathParser path_parser(query);
      auto path = path_parser.parse();

      if (!path) {
        last_error_ = "JSONPath parse error: " + path_parser.get_error();
        return "null";
      }

      result = JsonPathEngine::execute(path.get(), doc);
    } else {
      if (auto op = QueryEngine::parse(query, last_error_)) {
        simdjson::ondemand::document
            doc_for_jq; // Use a different name to avoid conflict
        auto doc_err = parser.iterate(padded).get(doc_for_jq);
        if (doc_err) {
          last_error_ = "Valid JSON but simdjson error: " +
                        std::string(simdjson::error_message(doc_err));
          return "null"; // Return "null" for consistency with other errors
        }
        QueryEngine engine(vars);
        auto jq_result = engine.execute(*op, doc_for_jq);
        return jq_result.toJson(2);
      } else {
        // Parse error already set in last_error_ by QueryEngine::parse
        return "null";
      }
    }

    return result.toJson();

  } catch (const std::exception &e) {
    last_error_ = std::string("Error: ") + e.what();
    return "null";
  }
}

std::string JsonQuery::get_error() { return last_error_; }

bool JsonQuery::is_jsonpath(const std::string &query) {
  // JSONPath starts with $
  // jq-style starts with .
  return !query.empty() && query[0] == '$';
}

} // namespace colored_json
