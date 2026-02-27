#pragma once

#include "query_engine.hpp"
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <functional>

namespace colored_json {

// ============================================================================
// JSONPath AST Node Types
// ============================================================================

enum class JsonPathNodeType {
    Root,           // $
    Property,       // .name or ['name']
    Index,          // [0]
    Slice,          // [start:end:step]
    Wildcard,       // [*] or .*
    RecursiveDescent, // ..
    Union,          // [name1,name2] or [0,1,2]
    Filter          // [?(@.price < 10)]
};

// Forward declaration
struct JsonPathNode;

// Filter comparison operators
enum class FilterOp {
    Eq,     // ==
    Ne,     // !=
    Lt,     // <
    Le,     // <=
    Gt,     // >
    Ge,     // >=
    And,    // &&
    Or,     // ||
    In,     // in
    Contains, // contains (for strings/arrays)
    Regex   // =~
};

// Filter expression node
struct FilterExpr {
    enum class Type {
        Comparison,     // @.price < 10
        Logical,        // expr && expr, expr || expr
        Literal,        // 10, "string", true
        Path,           // @.price, @
        Exists,         // @.name (just check if exists)
        Function        // length(@.name), etc.
    };
    
    Type type;
    
    // For comparison
    FilterOp op = FilterOp::Eq;
    std::unique_ptr<FilterExpr> left;
    std::unique_ptr<FilterExpr> right;
    
    // For literals
    QueryValue value;
    
    // For paths
    std::string path;
    
    // For functions
    std::string func_name;
    std::vector<std::unique_ptr<FilterExpr>> args;
    
    FilterExpr(Type t) : type(t) {}
};

// JSONPath AST Node
struct JsonPathNode {
    JsonPathNodeType type;
    
    // For Property
    std::string property_name;
    
    // For Index
    int index = 0;
    
    // For Slice
    int slice_start = 0;
    int slice_end = -1;   // -1 means "to end"
    int slice_step = 1;
    
    // For Union
    std::vector<std::variant<std::string, int>> union_items;
    
    // For Filter
    std::unique_ptr<FilterExpr> filter_expr;
    
    // Next node in path
    std::unique_ptr<JsonPathNode> next;
    
    explicit JsonPathNode(JsonPathNodeType t) : type(t) {}
};

// ============================================================================
// JSONPath Parser
// ============================================================================

class JsonPathParser {
public:
    explicit JsonPathParser(const std::string& path);
    
    std::unique_ptr<JsonPathNode> parse();
    std::string get_error() const { return error_; }

private:
    std::string path_;
    size_t pos_;
    std::string error_;
    
    // Helper methods
    bool at_end() const { return pos_ >= path_.size(); }
    char peek() const { return at_end() ? '\0' : path_[pos_]; }
    char get() { return at_end() ? '\0' : path_[pos_++]; }
    bool consume(char c);
    void skip_ws();
    
    // Parse methods
    std::unique_ptr<JsonPathNode> parse_root();
    std::unique_ptr<JsonPathNode> parse_path(std::unique_ptr<JsonPathNode> current);
    std::unique_ptr<JsonPathNode> parse_bracket(std::unique_ptr<JsonPathNode> current);
    std::unique_ptr<JsonPathNode> parse_dot(std::unique_ptr<JsonPathNode> current);
    std::unique_ptr<JsonPathNode> parse_recursive(std::unique_ptr<JsonPathNode> current);
    
    // Filter expression parsing
    std::unique_ptr<FilterExpr> parse_filter_expr();
    std::unique_ptr<FilterExpr> parse_filter_or();
    std::unique_ptr<FilterExpr> parse_filter_and();
    std::unique_ptr<FilterExpr> parse_filter_comparison();
    std::unique_ptr<FilterExpr> parse_filter_primary();
    std::unique_ptr<FilterExpr> parse_filter_literal();
    
    // Utility
    std::string parse_identifier();
    std::string parse_string_literal();
    int parse_integer();
};

// ============================================================================
// JSONPath Executor
// ============================================================================

class JsonPathEngine {
public:
    // Execute a JSONPath query
    static QueryValue execute(const JsonPathNode* path, const QueryValue& root);
    static QueryValue execute(const JsonPathNode* path, simdjson::ondemand::document& doc);
    
    // String interface for convenience
    static std::string execute_string(const std::string& json, const std::string& path);

private:
    // Execution methods
    static std::vector<QueryValue> evaluate_node(const JsonPathNode* node, const QueryValue& context);
    static std::vector<QueryValue> evaluate_recursive(const JsonPathNode* next, const QueryValue& context);
    static std::vector<QueryValue> evaluate_recursive_with_filter(const JsonPathNode* target, const QueryValue& context, const JsonPathNode* filter_node, const QueryValue& root);
    
    // Filter evaluation
    static bool evaluate_filter(const FilterExpr* expr, const QueryValue& context, const QueryValue& root);
    static QueryValue evaluate_filter_value(const FilterExpr* expr, const QueryValue& context, const QueryValue& root);
    
    // Helper for recursive descent
    static void collect_all(const QueryValue& value, std::vector<QueryValue>& results);
    static void collect_by_name(const QueryValue& value, const std::string& name, std::vector<QueryValue>& results);
};

// ============================================================================
// Unified Query Interface
// ============================================================================

class JsonQuery {
public:
    // Detect query type and execute
    static std::string execute(const std::string& json, const std::string& query);
    static std::string get_error();
    
    // Check if query is JSONPath
    static bool is_jsonpath(const std::string& query);
    
private:
    static std::string last_error_;
};

} // namespace colored_json
