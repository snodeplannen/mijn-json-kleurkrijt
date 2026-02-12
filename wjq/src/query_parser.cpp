#include "query_engine.hpp"
#include <cctype>
#include <stdexcept>

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
      error_msg_ = "Unexpected characters at position " + std::to_string(position_);
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
    if (at_end()) return '\0';
    return query_str_[position_];
  }

  char get_ch() {
    if (at_end()) return '\0';
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

  std::unique_ptr<QueryOp> parse_expr() {
    return parse_pipe_op();
  }

  std::unique_ptr<QueryOp> parse_pipe_op() {
    auto left = parse_or_op();
    
    while (true) {
      skip_ws();
      if (match_str("|")) {
        auto right = parse_or_op();
        left = QueryOp::pipe(std::move(left), std::move(right));
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
        left = QueryOp::binary(QueryOpType::Or, std::move(left), std::move(right));
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
        left = QueryOp::binary(QueryOpType::And, std::move(left), std::move(right));
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
        left = QueryOp::binary(QueryOpType::Equal, std::move(left), std::move(right));
      } else if (match_str("!=")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::NotEqual, std::move(left), std::move(right));
      } else if (match_str("<=")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::LessEqual, std::move(left), std::move(right));
      } else if (match_str(">=")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::GreaterEqual, std::move(left), std::move(right));
      } else if (match_str("<")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::Less, std::move(left), std::move(right));
      } else if (match_str(">")) {
        auto right = parse_add_sub_op();
        left = QueryOp::binary(QueryOpType::Greater, std::move(left), std::move(right));
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
        left = QueryOp::binary(QueryOpType::Add, std::move(left), std::move(right));
      } else if (match_str("-")) {
        auto right = parse_mul_div_op();
        left = QueryOp::binary(QueryOpType::Subtract, std::move(left), std::move(right));
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
        left = QueryOp::binary(QueryOpType::Multiply, std::move(left), std::move(right));
      } else if (match_str("/")) {
        auto right = parse_unary_op();
        left = QueryOp::binary(QueryOpType::Divide, std::move(left), std::move(right));
      } else if (match_str("%")) {
        auto right = parse_unary_op();
        left = QueryOp::binary(QueryOpType::Modulo, std::move(left), std::move(right));
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
      
      if (match_str(".")) {
        if (match_str(".")) {
          auto recursiveOp = QueryOp::recursive();
          skip_ws();
          if (!at_end() && peek_ch() != '|' && peek_ch() != ')' && peek_ch() != ']') {
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
            
            expr = QueryOp::pipe(std::move(expr), QueryOp::slice(start, end));
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
    
    if (match_str(".")) {
      if (match_str(".")) {
        return QueryOp::recursive();
      }
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
        if (ident == "select") funcType = QueryOpType::Select;
        else if (ident == "map") funcType = QueryOpType::Map;
        else if (ident == "keys") funcType = QueryOpType::Keys;
        else if (ident == "values") funcType = QueryOpType::Values;
        else if (ident == "length") funcType = QueryOpType::Length;
        else if (ident == "sort") funcType = QueryOpType::Sort;
        else if (ident == "unique") funcType = QueryOpType::Unique;
        else if (ident == "reverse") funcType = QueryOpType::Reverse;
        else if (ident == "contains") funcType = QueryOpType::Contains;
        else if (ident == "has") funcType = QueryOpType::Has;
        else {
          error_msg_ = "Unknown function: " + ident;
          return nullptr;
        }
        
        return QueryOp::function(funcType, std::move(args));
      }
      
      return QueryOp::property(ident);
    }
    
    return parse_literal_val();
  }

  std::unique_ptr<QueryOp> parse_prop_access() {
    skip_ws();
    
    if (match_str("\"")) {
      std::string name = parse_string_lit();
      return QueryOp::property(name);
    }
    
    if (match_str("[")) {
      std::string name = parse_string_lit();
      if (!consume_ch(']')) {
        error_msg_ = "Expected ']'";
        return nullptr;
      }
      return QueryOp::property(name);
    }
    
    std::string ident = parse_ident_name();
    if (ident.empty()) {
      return QueryOp::identity();
    }
    
    if (ident == "keys") return QueryOp::function(QueryOpType::Keys, {});
    if (ident == "values") return QueryOp::function(QueryOpType::Values, {});
    if (ident == "length") return QueryOp::function(QueryOpType::Length, {});
    if (ident == "sort") return QueryOp::function(QueryOpType::Sort, {});
    if (ident == "unique") return QueryOp::function(QueryOpType::Unique, {});
    if (ident == "reverse") return QueryOp::function(QueryOpType::Reverse, {});
    
    return QueryOp::property(ident);
  }

  std::unique_ptr<QueryOp> parse_literal_val() {
    skip_ws();
    
    if (match_str("\"")) {
      return QueryOp::literal(QueryValue(parse_string_lit()));
    }
    
    if (std::isdigit(peek_ch()) || (peek_ch() == '-' && std::isdigit(query_str_[position_ + 1]))) {
      std::string num;
      if (peek_ch() == '-') num += get_ch();
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
      std::vector<QueryValue> arr;
      skip_ws();
      if (peek_ch() != ']') {
        auto elem = parse_expr();
        if (elem->type == QueryOpType::Literal) {
          arr.push_back(elem->literalValue);
        }
        while (match_str(",")) {
          elem = parse_expr();
          if (elem->type == QueryOpType::Literal) {
            arr.push_back(elem->literalValue);
          }
        }
      }
      if (!consume_ch(']')) {
        error_msg_ = "Expected ']'";
        return nullptr;
      }
      return QueryOp::literal(QueryValue::array(std::move(arr)));
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
          case '/': result += c; break;
          case 'b': result += '\b'; break;
          case 'f': result += '\f'; break;
          case 'n': result += '\n'; break;
          case 'r': result += '\r'; break;
          case 't': result += '\t'; break;
          default: result += c;
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
    while (!at_end() && (std::isalnum(peek_ch()) || peek_ch() == '_' || peek_ch() == '-')) {
      result += get_ch();
    }
    return result;
  }
};

// Factory function
std::unique_ptr<JqQueryParser> createQueryParser(const std::string &query_str) {
  return std::make_unique<JqQueryParser>(query_str);
}

} // namespace colored_json
