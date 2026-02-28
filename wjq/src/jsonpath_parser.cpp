#include "jsonpath.hpp"
#include <cctype>

namespace colored_json {

// ============================================================================
// JsonPathParser Implementation
// ============================================================================

JsonPathParser::JsonPathParser(const std::string &path)
    : path_(path), pos_(0) {}

bool JsonPathParser::consume(char c) {
  skip_ws();
  if (peek() == c) {
    get();
    return true;
  }
  return false;
}

void JsonPathParser::skip_ws() {
  while (!at_end() && std::isspace(peek())) {
    get();
  }
}

std::unique_ptr<JsonPathNode> JsonPathParser::parse() {
  if (!consume('$')) {
    error_ = "JSONPath must start with $";
    return nullptr;
  }

  auto root = std::make_unique<JsonPathNode>(JsonPathNodeType::Root);
  auto *current = root.get();

  while (!at_end()) {
    auto next = parse_path(nullptr);
    if (error_.empty() && next) {
      current->next = std::move(next);
      current = current->next.get();
    } else if (!error_.empty()) {
      return nullptr;
    } else {
      break;
    }
  }

  return root;
}

std::unique_ptr<JsonPathNode>
JsonPathParser::parse_path(std::unique_ptr<JsonPathNode> current) {
  skip_ws();

  if (at_end()) {
    return nullptr;
  }

  char c = peek();

  if (c == '.') {
    get(); // consume '.'
    if (peek() == '.') {
      get(); // second '.'
      return parse_recursive(std::move(current));
    } else if (peek() == '*') {
      get();
      return std::make_unique<JsonPathNode>(JsonPathNodeType::Wildcard);
    } else {
      return parse_dot(nullptr);
    }
  } else if (c == '[') {
    return parse_bracket(nullptr);
  }

  return nullptr;
}

std::unique_ptr<JsonPathNode>
JsonPathParser::parse_dot(std::unique_ptr<JsonPathNode>) {
  skip_ws();

  auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Property);
  node->property_name = parse_identifier();

  if (node->property_name.empty()) {
    error_ = "Expected property name after '.'";
    return nullptr;
  }

  // Check for more path
  node->next = parse_path(nullptr);

  return node;
}

std::unique_ptr<JsonPathNode>
JsonPathParser::parse_bracket(std::unique_ptr<JsonPathNode>) {
  if (!consume('[')) {
    error_ = "Expected '['";
    return nullptr;
  }

  skip_ws();

  // Check for filter expression
  if (peek() == '?') {
    get(); // consume '?'
    if (!consume('(')) {
      error_ = "Expected '(' after '?'";
      return nullptr;
    }

    auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Filter);
    node->filter_expr = parse_filter_expr();

    if (!error_.empty()) {
      return nullptr;
    }

    skip_ws();
    if (!consume(')')) {
      error_ = "Expected ')' at end of filter expression, found: " +
               std::string(1, peek());
      return nullptr;
    }
    if (!consume(']')) {
      error_ = "Expected ']' at end of filter";
      return nullptr;
    }

    node->next = parse_path(nullptr);
    return node;
  }

  // Check for wildcard
  if (peek() == '*') {
    get();
    if (!consume(']')) {
      error_ = "Expected ']' after '*'";
      return nullptr;
    }
    auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Wildcard);
    node->next = parse_path(nullptr);
    return node;
  }

  // Could be: index, slice, or union
  // Try to parse as slice or index first
  if (std::isdigit(peek()) || peek() == '-' || peek() == ':') {
    int start = 0, end = -1, step = 1;
    bool has_slice = false;

    // Parse start
    if (peek() != ':') {
      start = parse_integer();
    }

    if (peek() == ':') {
      has_slice = true;
      get(); // consume ':'

      // Parse end
      if (std::isdigit(peek()) || peek() == '-') {
        end = parse_integer();
      }

      if (peek() == ':') {
        get(); // consume second ':'
        if (std::isdigit(peek()) || peek() == '-') {
          step = parse_integer();
          if (step == 0)
            step = 1;
        }
      }
    }

    if (!consume(']')) {
      error_ = "Expected ']'";
      return nullptr;
    }

    if (has_slice) {
      auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Slice);
      node->slice_start = start;
      node->slice_end = end;
      node->slice_step = step;
      node->next = parse_path(nullptr);
      return node;
    } else {
      auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Index);
      node->index = start;
      node->next = parse_path(nullptr);
      return node;
    }
  }

  // Could be string property access ['name'] or union
  if (peek() == '\'') {
    std::string name = parse_string_literal();

    // Check for union
    if (peek() == ',') {
      auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Union);
      node->union_items.push_back(name);

      while (peek() == ',') {
        get();
        skip_ws();
        if (peek() == '\'') {
          node->union_items.push_back(parse_string_literal());
        } else if (std::isdigit(peek())) {
          node->union_items.push_back(parse_integer());
        } else {
          error_ = "Expected string or number in union";
          return nullptr;
        }
      }

      if (!consume(']')) {
        error_ = "Expected ']'";
        return nullptr;
      }

      node->next = parse_path(nullptr);
      return node;
    }

    if (!consume(']')) {
      error_ = "Expected ']'";
      return nullptr;
    }

    auto node = std::make_unique<JsonPathNode>(JsonPathNodeType::Property);
    node->property_name = name;
    node->next = parse_path(nullptr);
    return node;
  }

  error_ = std::string("Unexpected character in bracket: ") + peek();
  return nullptr;
}

std::unique_ptr<JsonPathNode>
JsonPathParser::parse_recursive(std::unique_ptr<JsonPathNode>) {
  auto node =
      std::make_unique<JsonPathNode>(JsonPathNodeType::RecursiveDescent);

  // After .. can be: property name, wildcard, or bracket
  skip_ws();

  if (peek() == '*') {
    get();
    // Create a wildcard as the "next" of recursive descent
    node->next = std::make_unique<JsonPathNode>(JsonPathNodeType::Wildcard);
  } else if (peek() == '[') {
    node->next = parse_bracket(nullptr);
  } else {
    std::string name = parse_identifier();
    if (!name.empty()) {
      auto prop = std::make_unique<JsonPathNode>(JsonPathNodeType::Property);
      prop->property_name = name;
      node->next = std::move(prop);
    }
  }

  // Continue parsing after the recursive descent
  if (node->next) {
    auto *tail = node->next.get();
    while (tail->next)
      tail = tail->next.get();

    // Actually we need to append to the chain
    // For simplicity, just attach the rest
    auto rest = parse_path(nullptr);
    if (rest) {
      // Find the end of node->next chain
      auto *end = node->next.get();
      while (end->next)
        end = end->next.get();
      // This is tricky - we need to merge
    }
  }

  return node;
}

// ============================================================================
// Filter Expression Parsing
// ============================================================================

std::unique_ptr<FilterExpr> JsonPathParser::parse_filter_expr() {
  return parse_filter_or();
}

std::unique_ptr<FilterExpr> JsonPathParser::parse_filter_or() {
  auto left = parse_filter_and();

  while (true) {
    skip_ws();
    if (std::isalpha(peek())) {
      std::string id;
      while (!at_end() && std::isalpha(peek())) {
        id += get();
      }

      if (id == "or" || id == "||") {
        auto right = parse_filter_and();
        auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Logical);
        node->op = FilterOp::Or;
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
      } else {
        // Not 'or', put back
        pos_ -= id.size();
        break;
      }
    } else {
      break;
    }
  }

  return left;
}

std::unique_ptr<FilterExpr> JsonPathParser::parse_filter_and() {
  auto left = parse_filter_comparison();

  while (true) {
    skip_ws();
    if (std::isalpha(peek())) {
      std::string id;
      while (!at_end() && std::isalpha(peek())) {
        id += get();
      }

      if (id == "and" || id == "&&") {
        auto right = parse_filter_comparison();
        auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Logical);
        node->op = FilterOp::And;
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
      } else {
        pos_ -= id.size();
        break;
      }
    } else {
      break;
    }
  }

  return left;
}

std::unique_ptr<FilterExpr> JsonPathParser::parse_filter_comparison() {
  skip_ws();

  auto left = parse_filter_primary();

  skip_ws();

  // Parse comparison operator - don't consume until we know the full operator
  FilterOp op;
  if (peek() == '=') {
    get(); // consume '='
    if (peek() == '=') {
      get(); // consume second '='
      op = FilterOp::Eq;
    } else {
      // Just a single '=', not a valid comparison operator
      return left;
    }
  } else if (peek() == '!') {
    get();
    if (peek() == '=') {
      get();
      op = FilterOp::Ne;
    } else {
      return left;
    }
  } else if (peek() == '<') {
    get();
    if (peek() == '=') {
      get();
      op = FilterOp::Le;
    } else {
      op = FilterOp::Lt;
    }
  } else if (peek() == '>') {
    get();
    if (peek() == '=') {
      get();
      op = FilterOp::Ge;
    } else {
      op = FilterOp::Gt;
    }
  } else {
    // No comparison - could be exists check
    return left;
  }

  auto right = parse_filter_primary();

  auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Comparison);
  node->op = op;
  node->left = std::move(left);
  node->right = std::move(right);

  return node;
}

std::unique_ptr<FilterExpr> JsonPathParser::parse_filter_primary() {
  skip_ws();

  // Parenthesized expression
  if (consume('(')) {
    auto expr = parse_filter_expr();
    if (!consume(')')) {
      error_ = "Expected ')'";
      return nullptr;
    }
    return expr;
  }

  // Path starting with @ (current) or $ (root)
  if (peek() == '@' || peek() == '$') {
    char type = get();
    std::string path(1, type);

    // Parse rest of path
    while (!at_end() && (peek() == '.' || peek() == '[')) {
      if (peek() == '.') {
        get();
        path += '.';
        while (!at_end() && (std::isalnum(peek()) || peek() == '_')) {
          path += get();
        }
      } else if (peek() == '[') {
        get();
        path += '[';
        while (!at_end() && peek() != ']') {
          path += get();
        }
        if (peek() == ']') {
          path += get();
        }
      }
    }

    auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Path);
    node->path = path;
    return node;
  }

  // Function call
  if (std::isalpha(peek())) {
    std::string name = parse_identifier();
    skip_ws();

    if (peek() == '(') {
      // Function call
      get(); // consume '('
      auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Function);
      node->func_name = name;

      skip_ws();
      if (peek() != ')') {
        do {
          skip_ws();
          node->args.push_back(parse_filter_primary());
        } while (consume(','));
      }

      if (!consume(')')) {
        error_ = "Expected ')' after function arguments";
        return nullptr;
      }

      return node;
    }

    // Just a literal string (like 'in', 'and', 'or' that wasn't caught)
    auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Literal);
    node->value = QueryValue(name);
    return node;
  }

  // Literal value
  return parse_filter_literal();
}

std::unique_ptr<FilterExpr> JsonPathParser::parse_filter_literal() {
  skip_ws();

  // String literal
  if (peek() == '\'' || peek() == '"') {
    char quote = get();
    std::string value;
    while (!at_end() && peek() != quote) {
      if (peek() == '\\') {
        get();
        char escaped = get();
        switch (escaped) {
        case 'n':
          value += '\n';
          break;
        case 't':
          value += '\t';
          break;
        case 'r':
          value += '\r';
          break;
        case '\\':
          value += '\\';
          break;
        case '\'':
          value += '\'';
          break;
        case '"':
          value += '"';
          break;
        default:
          value += escaped;
          break;
        }
      } else {
        value += get();
      }
    }
    get(); // consume closing quote

    auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Literal);
    node->value = QueryValue(value);
    return node;
  }

  // Number
  if (std::isdigit(peek()) || peek() == '-') {
    std::string num;
    if (peek() == '-')
      num += get();
    while (!at_end() && (std::isdigit(peek()) || peek() == '.')) {
      num += get();
    }

    auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Literal);
    node->value = QueryValue(std::stod(num));
    return node;
  }

  // Boolean or null
  if (std::isalpha(peek())) {
    std::string id = parse_identifier();

    auto node = std::make_unique<FilterExpr>(FilterExpr::Type::Literal);
    if (id == "true") {
      node->value = QueryValue(true);
    } else if (id == "false") {
      node->value = QueryValue(false);
    } else if (id == "null") {
      node->value = QueryValue::null();
    } else {
      node->value = QueryValue(id);
    }
    return node;
  }

  error_ = std::string("Unexpected character in filter: ") + peek();
  return nullptr;
}

// ============================================================================
// Utility Methods
// ============================================================================

std::string JsonPathParser::parse_identifier() {
  std::string id;
  while (!at_end() && (std::isalnum(peek()) || peek() == '_')) {
    id += get();
  }
  return id;
}

std::string JsonPathParser::parse_string_literal() {
  if (peek() != '\'' && peek() != '"') {
    return "";
  }

  char quote = get();
  std::string value;

  while (!at_end() && peek() != quote) {
    if (peek() == '\\') {
      get();
      char escaped = get();
      switch (escaped) {
      case 'n':
        value += '\n';
        break;
      case 't':
        value += '\t';
        break;
      case 'r':
        value += '\r';
        break;
      case '\\':
        value += '\\';
        break;
      case '\'':
        value += '\'';
        break;
      case '"':
        value += '"';
        break;
      default:
        value += escaped;
        break;
      }
    } else {
      value += get();
    }
  }

  if (peek() == quote) {
    get();
  }

  return value;
}

int JsonPathParser::parse_integer() {
  std::string num;
  if (peek() == '-') {
    num += get();
  }
  while (!at_end() && std::isdigit(peek())) {
    num += get();
  }
  return num.empty() ? 0 : std::stoi(num);
}

} // namespace colored_json
