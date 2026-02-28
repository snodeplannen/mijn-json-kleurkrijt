#include "style.hpp"
#include "themes.hpp"
#include <catch2/catch_test_macros.hpp>


using namespace colored_json;

TEST_CASE("Style basic construction", "[style]") {
  Style s;

  REQUIRE(s.key_color == Color(93, 173, 226));
  REQUIRE(s.string_color == Color(152, 224, 36));
  REQUIRE(s.number_color == Color(255, 203, 107));
  REQUIRE(s.color_mode == ColorMode::Auto);
  REQUIRE(s.compact == false);
  REQUIRE(s.indent_size == 2);
}

TEST_CASE("Style get_base_color", "[style]") {
  Style s;

  REQUIRE(s.get_base_color(ElementType::Key) == s.key_color);
  REQUIRE(s.get_base_color(ElementType::String) == s.string_color);
  REQUIRE(s.get_base_color(ElementType::Number) == s.number_color);
  REQUIRE(s.get_base_color(ElementType::Boolean) == s.bool_color);
  REQUIRE(s.get_base_color(ElementType::Null) == s.null_color);
}

TEST_CASE("Style get_color with context", "[style]") {
  Style s;
  StyleContext ctx;

  REQUIRE(s.get_color(ElementType::Key, ctx) == s.key_color);
  REQUIRE(s.get_color(ElementType::String, ctx, "test") == s.string_color);
}

TEST_CASE("Style legacy key_colors", "[style]") {
  Style s;
  s.setKeyColor("name", Color(255, 0, 0));

  StyleContext ctx;
  ctx.key_name = "name";

  Color result = s.get_color(ElementType::Key, ctx);
  REQUIRE(result == Color(255, 0, 0));
}

TEST_CASE("Style rule creation", "[style]") {
  Style s;

  auto &rule = s.add_rule();
  rule.for_elements({ElementType::String})
      .when_value(keyword({"error"}))
      .use_color(Color(255, 0, 0))
      .with_priority(10);

  REQUIRE(s.rules.size() == 1);
  REQUIRE(s.rules[0]->priority == 10);
  REQUIRE(s.rules[0]->element_types.size() == 1);
  REQUIRE(s.rules[0]->element_types[0] == ElementType::String);
}

TEST_CASE("StyleRule matches element type", "[style]") {
  StyleRule rule;
  rule.for_elements({ElementType::String, ElementType::Number});

  StyleContext ctx;

  REQUIRE(rule.matches(ElementType::String, ctx) == true);
  REQUIRE(rule.matches(ElementType::Number, ctx) == true);
  REQUIRE(rule.matches(ElementType::Key, ctx) == false);
}

TEST_CASE("StyleRule matches path", "[style]") {
  StyleRule rule;
  rule.at_path("user.*");

  StyleContext ctx;
  ctx.path = "user.name";

  REQUIRE(rule.matches(ElementType::String, ctx) == true);

  ctx.path = "other.name";
  REQUIRE(rule.matches(ElementType::String, ctx) == false);
}

TEST_CASE("StyleRule matches depth", "[style]") {
  StyleRule rule;
  rule.at_depth(1, 2);

  StyleContext ctx;

  ctx.depth = 0;
  REQUIRE(rule.matches(ElementType::String, ctx) == false);

  ctx.depth = 1;
  REQUIRE(rule.matches(ElementType::String, ctx) == true);

  ctx.depth = 2;
  REQUIRE(rule.matches(ElementType::String, ctx) == true);

  ctx.depth = 3;
  REQUIRE(rule.matches(ElementType::String, ctx) == false);
}

TEST_CASE("StyleRule matches array index", "[style]") {
  StyleRule rule;
  rule.at_array_index("even");

  StyleContext ctx;
  ctx.array_index = 0;
  REQUIRE(rule.matches(ElementType::Number, ctx) == true);

  ctx.array_index = 1;
  REQUIRE(rule.matches(ElementType::Number, ctx) == false);
}

TEST_CASE("StyleRule value matching", "[style]") {
  StyleRule rule;
  rule.when_value(keyword({"error", "warning"}));

  StyleContext ctx;

  REQUIRE(rule.matches_value("error", ctx) == true);
  REQUIRE(rule.matches_value("warning", ctx) == true);
  REQUIRE(rule.matches_value("success", ctx) == false);
}

TEST_CASE("StyleRule clone", "[style]") {
  StyleRule rule;
  rule.for_elements({ElementType::String})
      .use_color(Color(255, 0, 0))
      .with_priority(5);

  auto clone = rule.clone();
  REQUIRE(clone->priority == rule.priority);
  REQUIRE(clone->color == rule.color);
}

TEST_CASE("Theme presets exist", "[style]") {
  auto presets = Style::listPresets();
  REQUIRE(presets.size() >= 7); // At least original 7 themes

  // Check specific themes exist
  REQUIRE(std::find(presets.begin(), presets.end(), "default") !=
          presets.end());
  REQUIRE(std::find(presets.begin(), presets.end(), "dracula") !=
          presets.end());
  REQUIRE(std::find(presets.begin(), presets.end(), "monokai") !=
          presets.end());
}

TEST_CASE("Theme loading", "[style]") {
  auto s1 = Style::getPreset("default");
  REQUIRE(s1.key_color.r > 0); // Should have actual colors

  auto s2 = Style::getPreset("monokai");
  REQUIRE(s2.key_color != s1.key_color); // Different themes should differ

  // Note: dracula and default are now the same theme, test with another theme
  auto s3 = Style::getPreset("neon");
  REQUIRE(s3.string_color != s1.string_color);
}

TEST_CASE("Theme invalid preset throws", "[style]") {
  REQUIRE_THROWS(Style::getPreset("nonexistent_theme"));
}

TEST_CASE("Style builder pattern", "[style]") {
  Style s;
  s.with_compact(true).with_indent(4).with_color_mode(ColorMode::TrueColor);

  REQUIRE(s.compact == true);
  REQUIRE(s.indent_size == 4);
  REQUIRE(s.color_mode == ColorMode::TrueColor);
}

TEST_CASE("Advanced themes", "[style]") {
  SECTION("Debug theme has rules") {
    auto debug = make_debug_theme();
    REQUIRE(debug.rules.size() > 0);
  }

  SECTION("Depth-aware theme has rules") {
    auto depth = make_depth_aware_theme();
    REQUIRE(depth.rules.size() > 0);
  }

  SECTION("Data-analysis theme has rules") {
    auto data = make_data_analysis_theme();
    REQUIRE(data.rules.size() > 0);
  }
}

TEST_CASE("Debug theme has rules helper", "[style]") {
  REQUIRE(debug_theme_has_rules() == true);
}

TEST_CASE("Rule priority ordering", "[style]") {
  Style s;

  // Add low priority rule
  s.add_rule()
      .for_elements({ElementType::String})
      .use_color(Color(255, 0, 0))
      .with_priority(1);

  // Add high priority rule
  s.add_rule()
      .for_elements({ElementType::String})
      .use_color(Color(0, 255, 0))
      .with_priority(10);

  StyleContext ctx;
  ctx.path = "";

  Color result = s.get_color(ElementType::String, ctx);
  // Higher priority (10) should win
  REQUIRE(result == Color(0, 255, 0));
}

TEST_CASE("Themes have boolean coloring rules", "[style]") {
  auto default_theme = Style::getPreset("default");

  // We expect rules for booleans to exist in the default theme
  REQUIRE(default_theme.rules.size() >= 2);

  StyleContext ctx;
  ctx.path = "";

  Color green(50, 255, 50);
  Color red(255, 50, 50);

  REQUIRE(default_theme.get_color(ElementType::Boolean, ctx, "true") == green);
  REQUIRE(default_theme.get_color(ElementType::Boolean, ctx, "false") == red);
}
