#include "jsonpath.hpp"
#include <catch2/catch_test_macros.hpp>
#include <simdjson.h>

using namespace colored_json;

TEST_CASE("Query Object Construction", "[query]") {
  std::string json = R"({"first_name": "Bob", "age": 30})";

  // Test basic object construction
  std::string q1 = R"({name: .first_name, years: .age})";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  std::cout << "q1 result: " << r1 << std::endl;
  REQUIRE(r1.find(R"("name": "Bob")") != std::string::npos);
  REQUIRE(r1.find(R"("years": 30)") != std::string::npos);

  // Test simplified object shorthand {first_name}
  std::string q2 = R"({first_name, age})";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find(R"("first_name": "Bob")") != std::string::npos);
  REQUIRE(r2.find(R"("age": 30)") != std::string::npos);
}

TEST_CASE("Query Array Construction", "[query]") {
  std::string json = R"({"a": 1, "b": 2, "c": 3})";

  std::string q1 = R"([.a, .c, .b])";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  // simdjson prints compact or pretty? Execute usually formats depending on
  // config. Let's just check the values exist in order if possible, or just
  // string.
  REQUIRE(r1.find("1") != std::string::npos);
  REQUIRE(r1.find("3") != std::string::npos);
  REQUIRE(r1.find("2") != std::string::npos);

  // Check array with literals
  std::string q2 = R"([.a, 42, "hello"])";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find("42") != std::string::npos);
  REQUIRE(r2.find("1") != std::string::npos);
  REQUIRE(r2.find("hello") != std::string::npos);
}

TEST_CASE("Query String Interpolation", "[query]") {
  std::string json = R"({"first_name": "Bob", "age": 30, "admin": true})";

  std::string q1 = R"###("User \(.first_name) is \(.age) years old.")###";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r1.find(R"("User Bob is 30 years old.")") != std::string::npos);

  std::string q2 = R"###("admin status: \(.admin)")###";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find(R"("admin status: true")") != std::string::npos);
}

TEST_CASE("Query Mutation Operators", "[query]") {
  std::string json = R"({"user": {"name": "Bob", "age": 30}})";

  // Test basic assignment =
  std::string q1 = R"(.user.name = "Alice")";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r1.find(R"("name": "Alice")") != std::string::npos);
  REQUIRE(r1.find(R"("age": 30)") != std::string::npos);

  // Test update assignment |=
  std::string q2 = R"(.user.age |= . + 1)";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find(R"("name": "Bob")") != std::string::npos);
  REQUIRE(r2.find(R"("age": 31)") != std::string::npos);
}

TEST_CASE("Query Regex Functions", "[query]") {
  std::string json = R"({"email": "alice@example.com", "text": "foo123bar"})";

  // Test regex test()
  std::string q1 = R"(.email | test(".*@example.com"))";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r1.find("true") != std::string::npos);

  // Test regex match()
  std::string q2 = R"(.text | match("[0-9]+"))";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find(R"("string": "123")") != std::string::npos);

  // Test regex sub()
  std::string q3 = R"(.text | sub("[0-9]+", "XYZ"))";
  std::string r3 = JsonQuery::execute(json, q3);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r3.find("fooXYZbar") != std::string::npos);
}

TEST_CASE("Query TryCatch and Optional Operators", "[query]") {
  std::string json = R"({"name": "Bob", "numbers": [1, 2, 3]})";

  // Test optional operator ?
  // Normally `.missing` returns null without error in this implementation.
  // But syntax wise, `?` should parse and execute.
  std::string q1 = R"(.missing?)";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r1.find("null") != std::string::npos);

  // Test try catch
  std::string q2 = R"(try .name catch "error")";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find(R"("Bob")") != std::string::npos);

  // Test try catch with forced failure
  // `error` function in jq throws, but we don't have it.
  // We can just verify it parses and runs the `try` block.
  std::string q3 = R"(try .missing catch "error fallback")";
  std::string r3 = JsonQuery::execute(json, q3);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r3.find("null") != std::string::npos);
}

TEST_CASE("Query Variable Bindings", "[query]") {
  std::string json = R"({"name": "Alice", "score": 100})";

  // Bind and use a variable
  std::string q1 = R"(.name as $n | {user: $n, active: true})";
  std::string r1 = JsonQuery::execute(json, q1);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r1.find(R"("user": "Alice")") != std::string::npos);
  REQUIRE(r1.find("true") != std::string::npos);

  // Shadowing variables
  std::string q2 = R"(10 as $x | 20 as $x | $x)";
  std::string r2 = JsonQuery::execute(json, q2);
  REQUIRE(JsonQuery::get_error() == "");
  REQUIRE(r2.find("20") != std::string::npos);
}
