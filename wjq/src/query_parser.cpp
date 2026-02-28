#include "query_engine.hpp"
#include <cctype>

namespace colored_json {

// ============================================================================
// JqQueryParser Definition and Implementation
// ============================================================================

class JqQueryParser {
public:
  explicit JqQueryParser(const std::string &query_str)
      : query_str_(query_str), position_(0) {}

  std::unique_ptr<QueryOp> parse() {
    skip_ws();
    auto result = parse_expr();
    skip_ws();
    if (!at_end()) {
      error_msg_ =
          "Unexpected characters at position " + std::to_string(position_);
    }
    return result;
  }

  std::string get_error() const { return error_msg_; }

private:
  std::string query_str_;
  size_t position_;
  std::string error_msg_;

  bool at_end() const { return position_ >= query_str_.size(); }

  void skip_ws() {
    while (!at_end() && std::isspace(peek_ch())) {
      get_ch();
    }
  }

  char peek_ch() const {
    if (at_end())
      return '\0';
    return query_str_[position_];
  }

  char get_ch() {
    if (at_end())
      return '\0';
    return query_str_[position_++];
  }

  bool consume_ch(char c) {
    skip_ws();
    if (peek_ch() == c) {
      get_ch();
      return true;
    }
    return false;
  }

  bool match_str(const std::string &s) {
    skip_ws();
    if (query_str_.compare(position_, s.size(), s) == 0) {
      position_ += s.size();
      return true;
    }
    return false;
  }

  std::unique_ptr<QueryOp> parse_expr() { return parse_pipe_op(); }

  std::unique_ptr<QueryOp> parse_pipe_op() {
    auto left = parse_assign_op();

    while (true) {
      skip_ws();
      if (match_str("as")) {
        skip_ws();
        if (!match_str("$")) {
          error_msg_ = "Expected '$' after 'as'";
          return nullptr;
        }
        std::string var_name = parse_ident_name();
        skip_ws();
        if (!match_str("|")) {
          error_msg_ = "Expected '|' after variable binding";
          return nullptr;
        }
        auto right = parse_pipe_op();
        auto op = std::make_unique<QueryOp>(QueryOpType::BindVariable);
        op->args.push_back(std::move(left));
        op->args.push_back(std::move(right));
        op->prop_name = var_name;
        return op;
      } else if (match_str("|")) {
        auto right = parse_assign_op();
        left = QueryOp::pipe(std::move(left), std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_assign_op() {
    auto left = parse_or_op();

    while (true) {
      skip_ws();
      if (match_str("=")) {
        // == is already handled in parse_compare_op, but watch out for =
        // matching == Actually, parse_compare_op has higher precedence and
        // parses == before we get here. Wait! If the string is ==,
        // match_str("=") matches the first =! We need to be careful.
        if (peek_ch() == '=') {
          // It's ==, not =. We shouldn't consume it here!
          break;
        }
        auto right = parse_or_op();
        left = QueryOp::binary(QueryOpType::Assign, std::move(left),
                               std::move(right));
      } else if (match_str("|=")) {
        auto right = parse_or_op();
        left = QueryOp::binary(QueryOpType::UpdateAssign, std::move(left),
                               std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_or_op() {
    auto left = parse_and_op();

    while (true) {
      skip_ws();
      if (match_str("or")) {
        auto right = parse_and_op();
        left =
            QueryOp::binary(QueryOpType::Or, std::move(left), std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_and_op() {
    auto left = parse_compare_op();

    while (true) {
      skip_ws();
      if (match_str("and")) {
        auto right = parse_compare_op();
        left = QueryOp::binary(QueryOpType::And, std::move(left),
                               std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_compare_op() {
    auto left = parse_add_sub_op();

    while (true) {
      skip_ws();
      if (match_str("==")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::Equal, std::move(left),
                               std::move(right));
      } else if (match_str("!=")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::NotEqual, std::move(left),
                               std::move(right));
      } else if (match_str("<=")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::LessEqual, std::move(left),
                               std::move(right));
      } else if (match_str(">=")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::GreaterEqual, std::move(left),
                               std::move(right));
      } else if (match_str("<")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::Less, std::move(left),
                               std::move(right));
      } else if (match_str(">")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::Greater, std::move(left),
                               std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_add_sub_op() {
    auto left = parse_mul_div_op();

    while (true) {
      skip_ws();
      if (match_str("+")) {
        auto right = parse_mul_div_op();
        left = QueryOp::binary(QueryOpType::Add, std::move(left),
                               std::move(right));
      } else if (match_str("-")) {
        auto right = parse_mul_div_op();
        left = QueryOp::binary(QueryOpType::Subtract, std::move(left),
                               std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_mul_div_op() {
    auto left = parse_unary_op();

    while (true) {
      skip_ws();
      if (match_str("*")) {
        auto right = parse_unary_op();
        left = QueryOp::binary(QueryOpType::Multiply, std::move(left),
                               std::move(right));
      } else if (match_str("/")) {
        auto right = parse_unary_op();
        left = QueryOp::binary(QueryOpType::Divide, std::move(left),
                               std::move(right));
      } else if (match_str("%")) {
        auto right = parse_unary_op();
        left = QueryOp::binary(QueryOpType::Modulo, std::move(left),
                               std::move(right));
      } else {
        break;
      }
    }

    return left;
  }

  std::unique_ptr<QueryOp> parse_unary_op() {
    skip_ws();

    if (match_str("not")) {
      auto arg = parse_unary_op();
      return QueryOp::unary(QueryOpType::Not, std::move(arg));
    }

    if (match_str("-")) {
      auto arg = parse_unary_op();
      return QueryOp::unary(QueryOpType::Not, std::move(arg));
    }

    return parse_postfix_op();
  }

  std::unique_ptr<QueryOp> parse_postfix_op() {
    auto expr = parse_primary_op();

    while (true) {
      skip_ws();

      if (match_str("?")) {
        expr = QueryOp::unary(QueryOpType::Optional, std::move(expr));
        continue;
      }

      if (match_str(".")) {
        if (match_str(".")) {
          auto recursiveOp = QueryOp::recursive();
          skip_ws();
          if (!at_end() && peek_ch() != '|' && peek_ch() != ')' &&
              peek_ch() != ']') {
            auto subquery = parse_prop_access();
            expr = QueryOp::pipe(std::move(expr), std::move(recursiveOp));
            expr = QueryOp::pipe(std::move(expr), std::move(subquery));
          } else {
            expr = QueryOp::pipe(std::move(expr), std::move(recursiveOp));
          }
        } else if (match_str("[")) {
          auto index = parse_expr();
          if (!consume_ch(']')) {
            error_msg_ = "Expected ']'";
            return nullptr;
          }
          expr = QueryOp::pipe(std::move(expr), std::move(index));
        } else if (match_str("*")) {
          expr = QueryOp::pipe(std::move(expr), QueryOp::iterator());
        } else {
          auto prop = parse_prop_access();
          if (prop) {
            expr = QueryOp::pipe(std::move(expr), std::move(prop));
          }
        }
      } else if (match_str("[")) {
        skip_ws();
        if (match_str("]")) {
          expr = QueryOp::pipe(std::move(expr), QueryOp::iterator());
        } else {
          bool isSlice = false;
          size_t savePos = position_;

          while (!at_end() && (std::isdigit(peek_ch()) || peek_ch() == '-')) {
            get_ch();
          }
          if (peek_ch() == ':') {
            isSlice = true;
          }
          position_ = savePos;

          if (isSlice) {
            int64_t start = 0, end = INT64_MAX;

            if (peek_ch() != ':') {
              start = std::stoll(parse_ident_name());
              skip_ws();
            }

            if (!consume_ch(':')) {
              error_msg_ = "Expected ':' in slice";
              return nullptr;
            }

            skip_ws();
            if (peek_ch() != ']' && !at_end()) {
              end = std::stoll(parse_ident_name());
            }

            if (!consume_ch(']')) {
              error_msg_ = "Expected ']'";
              return nullptr;
            }

            expr =
                QueryOp::pipe(std::move(expr), QueryOp::make_slice(start, end));
          } else {
            auto indexExpr = parse_expr();
            if (!consume_ch(']')) {
              error_msg_ = "Expected ']'";
              return nullptr;
            }
            expr = QueryOp::pipe(std::move(expr), std::move(indexExpr));
          }
        }
      } else {
        break;
      }
    }

    return expr;
  }

  std::unique_ptr<QueryOp> parse_primary_op() {
    skip_ws();

    if (match_str("try")) {
      auto tryExpr = parse_expr();
      std::unique_ptr<QueryOp> catchExpr = nullptr;
      skip_ws();
      if (match_str("catch")) {
        catchExpr = parse_expr();
      } else {
        catchExpr = std::make_unique<QueryOp>(QueryOpType::Literal);
        catchExpr->literalValue = QueryValue::null();
      }
      auto op = std::make_unique<QueryOp>(QueryOpType::TryCatch);
      op->args.push_back(std::move(tryExpr));
      op->args.push_back(std::move(catchExpr));
      return op;
    }

    if (match_str(".")) {
      if (match_str(".")) {
        return QueryOp::recursive();
      }
      // Check if it's followed by a property name directly
      if (std::isalpha(peek_ch()) || peek_ch() == '_') {
        std::string ident = parse_ident_name();
        return QueryOp::make_property(ident);
      }
      // Check if it's followed by brackets like .[
      if (peek_ch() == '[') {
        return QueryOp::identity();
      }
      // Just . (identity)
      return QueryOp::identity();
    }

    if (consume_ch('(')) {
      auto expr = parse_expr();
      if (!consume_ch(')')) {
        error_msg_ = "Expected ')'";
        return nullptr;
      }
      return expr;
    }

    // Check for keyword literals BEFORE the general identifier branch
    // so that true, false, null are parsed as values, not property accesses
    {
      size_t saved = position_;
      skip_ws();
      if (query_str_.compare(position_, 4, "true") == 0 &&
          (position_ + 4 >= query_str_.size() ||
           !std::isalnum(query_str_[position_ + 4]))) {
        position_ += 4;
        return QueryOp::literal(QueryValue(true));
      }
      if (query_str_.compare(position_, 5, "false") == 0 &&
          (position_ + 5 >= query_str_.size() ||
           !std::isalnum(query_str_[position_ + 5]))) {
        position_ += 5;
        return QueryOp::literal(QueryValue(false));
      }
      if (query_str_.compare(position_, 4, "null") == 0 &&
          (position_ + 4 >= query_str_.size() ||
           !std::isalnum(query_str_[position_ + 4]))) {
        position_ += 4;
        return QueryOp::literal(QueryValue::null());
      }
      position_ = saved;
    }

    if (std::isalpha(peek_ch()) || peek_ch() == '_') {
      std::string ident = parse_ident_name();

      skip_ws();
      if (match_str("(")) {
        std::vector<std::unique_ptr<QueryOp>> args;

        skip_ws();
        if (peek_ch() != ')') {
          args.push_back(parse_expr());
          while (match_str(",")) {
            args.push_back(parse_expr());
          }
        }

        if (!consume_ch(')')) {
          error_msg_ = "Expected ')' after function arguments";
          return nullptr;
        }

        QueryOpType funcType;
        if (ident == "select")
          funcType = QueryOpType::Select;
        else if (ident == "map")
          funcType = QueryOpType::Map;
        else if (ident == "keys")
          funcType = QueryOpType::Keys;
        else if (ident == "values")
          funcType = QueryOpType::Values;
        else if (ident == "length")
          funcType = QueryOpType::Length;
        else if (ident == "sort")
          funcType = QueryOpType::Sort;
        else if (ident == "unique")
          funcType = QueryOpType::Unique;
        else if (ident == "reverse")
          funcType = QueryOpType::Reverse;
        else if (ident == "contains")
          funcType = QueryOpType::Contains;
        else if (ident == "has")
          funcType = QueryOpType::Has;
        else if (ident == "test")
          funcType = QueryOpType::Test;
        else if (ident == "match")
          funcType = QueryOpType::Match;
        else if (ident == "sub")
          funcType = QueryOpType::Sub;
        else {
          error_msg_ = "Unknown function: " + ident;
          return nullptr;
        }

        return QueryOp::function(funcType, std::move(args));
      }

      return QueryOp::make_property(ident);
    }

    return parse_literal_val();
  }

  std::unique_ptr<QueryOp> parse_prop_access() {
    skip_ws();

    if (match_str("\"")) {
      std::string name = parse_string_lit();
      return QueryOp::make_property(name);
    }

    if (match_str("[")) {
      std::string name = parse_string_lit();
      if (!consume_ch(']')) {
        error_msg_ = "Expected ']'";
        return nullptr;
      }
      return QueryOp::make_property(name);
    }

    std::string ident = parse_ident_name();
    if (ident.empty()) {
      return QueryOp::identity();
    }

    if (ident == "keys")
      return QueryOp::function(QueryOpType::Keys, {});
    if (ident == "values")
      return QueryOp::function(QueryOpType::Values, {});
    if (ident == "length")
      return QueryOp::function(QueryOpType::Length, {});
    if (ident == "sort")
      return QueryOp::function(QueryOpType::Sort, {});
    if (ident == "unique")
      return QueryOp::function(QueryOpType::Unique, {});
    if (ident == "reverse")
      return QueryOp::function(QueryOpType::Reverse, {});

    return QueryOp::make_property(ident);
  }

  std::unique_ptr<QueryOp> parse_interpolated_string() {
    std::vector<std::unique_ptr<QueryOp>> parts;
    std::string current_literal = "";

    while (!at_end() && peek_ch() != '"') {
      if (peek_ch() == '\\') {
        get_ch(); // consume '\'
        char c = peek_ch();
        if (c == '(') {
          get_ch(); // consume '('
          if (!current_literal.empty()) {
            parts.push_back(QueryOp::literal(QueryValue(current_literal)));
            current_literal = "";
          }
          auto expr = parse_expr();
          if (!consume_ch(')')) {
            error_msg_ = "Expected ')' after string interpolation";
            return nullptr;
          }
          parts.push_back(std::move(expr));
        } else {
          c = get_ch();
          switch (c) {
          case '"':
          case '\\':
          case '/':
            current_literal += c;
            break;
          case 'b':
            current_literal += '\b';
            break;
          case 'f':
            current_literal += '\f';
            break;
          case 'n':
            current_literal += '\n';
            break;
          case 'r':
            current_literal += '\r';
            break;
          case 't':
            current_literal += '\t';
            break;
          default:
            current_literal += c;
            break;
          }
        }
      } else {
        current_literal += get_ch();
      }
    }

    if (!match_str("\"")) {
      error_msg_ = "Unterminated string";
      return nullptr;
    }

    if (!current_literal.empty() || parts.empty()) {
      parts.push_back(QueryOp::literal(QueryValue(current_literal)));
    }

    if (parts.size() == 1 && parts[0]->type == QueryOpType::Literal) {
      return std::move(parts[0]);
    }

    return QueryOp::make_string_interpolation(std::move(parts));
  }

  std::unique_ptr<QueryOp> parse_literal_val() {
    skip_ws();

    if (match_str("$")) {
      std::string var_name = parse_ident_name();
      auto op = std::make_unique<QueryOp>(QueryOpType::Variable);
      op->prop_name = var_name;
      return op;
    }

    if (match_str("\"")) {
      return parse_interpolated_string();
    }

    if (std::isdigit(peek_ch()) ||
        (peek_ch() == '-' && std::isdigit(query_str_[position_ + 1]))) {
      std::string num;
      if (peek_ch() == '-')
        num += get_ch();
      while (std::isdigit(peek_ch())) {
        num += get_ch();
      }
      if (match_str(".")) {
        num += ".";
        while (std::isdigit(peek_ch())) {
          num += get_ch();
        }
      }
      return QueryOp::literal(QueryValue(std::stod(num)));
    }

    if (match_str("true")) {
      return QueryOp::literal(QueryValue(true));
    }
    if (match_str("false")) {
      return QueryOp::literal(QueryValue(false));
    }
    if (match_str("null")) {
      return QueryOp::literal(QueryValue::null());
    }

    if (consume_ch('[')) {
      std::vector<std::unique_ptr<QueryOp>> arr;
      skip_ws();
      if (peek_ch() != ']') {
        arr.push_back(parse_expr());
        while (match_str(",")) {
          arr.push_back(parse_expr());
        }
      }
      if (!consume_ch(']')) {
        error_msg_ = "Expected ']'";
        return nullptr;
      }
      return QueryOp::make_array(std::move(arr));
    }

    if (consume_ch('{')) {
      std::vector<std::string> keys;
      std::vector<std::unique_ptr<QueryOp>> vals;
      skip_ws();
      if (peek_ch() != '}') {
        do {
          skip_ws();
          std::string key;
          if (match_str("\"")) {
            key = parse_string_lit();
          } else {
            key = parse_ident_name();
            if (key.empty()) {
              error_msg_ = "Expected string or identifier as object key";
              return nullptr;
            }
          }
          skip_ws();

          if (match_str(":")) {
            keys.push_back(key);
            vals.push_back(parse_expr());
          } else {
            // simplified {key} shorthand for {key: .key}
            keys.push_back(key);
            vals.push_back(QueryOp::make_property(key));
          }
        } while (match_str(","));
      }
      if (!consume_ch('}')) {
        error_msg_ = "Expected '}'";
        return nullptr;
      }
      return QueryOp::make_object(std::move(keys), std::move(vals));
    }

    error_msg_ = "Unexpected character: " + std::string(1, peek_ch());
    return nullptr;
  }

  std::string parse_string_lit() {
    std::string result;
    while (!at_end() && peek_ch() != '"') {
      if (peek_ch() == '\\') {
        get_ch();
        char c = get_ch();
        switch (c) {
        case '"':
        case '\\':
        case '/':
          result += c;
          break;
        case 'b':
          result += '\b';
          break;
        case 'f':
          result += '\f';
          break;
        case 'n':
          result += '\n';
          break;
        case 'r':
          result += '\r';
          break;
        case 't':
          result += '\t';
          break;
        default:
          result += c;
        }
      } else {
        result += get_ch();
      }
    }
    if (!match_str("\"")) {
      error_msg_ = "Unterminated string";
    }
    return result;
  }

  std::string parse_ident_name() {
    std::string result;
    while (!at_end() &&
           (std::isalnum(peek_ch()) || peek_ch() == '_' || peek_ch() == '-')) {
      result += get_ch();
    }
    return result;
  }
};

// Factory function
std::unique_ptr<JqQueryParser> createQueryParser(const std::string &query_str) {
  return std::make_unique<JqQueryParser>(query_str);
}

std::unique_ptr<QueryOp> QueryEngine::parse(const std::string &query,
                                            std::string &error) {
  JqQueryParser parser(query);
  auto op = parser.parse();
  error = parser.get_error();
  return op;
}

} // namespace colored_json
