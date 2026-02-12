#include <catch2/catch_test_macros.hpp>
#include "color.hpp"

using namespace colored_json;

TEST_CASE("Color basic construction", "[color]") {
    Color c1;
    REQUIRE(c1.r == 0);
    REQUIRE(c1.g == 0);
    REQUIRE(c1.b == 0);

    Color c2(255, 128, 64);
    REQUIRE(c2.r == 255);
    REQUIRE(c2.g == 128);
    REQUIRE(c2.b == 64);

    Color c3 = Color::fromRgb(100, 150, 200);
    REQUIRE(c3.r == 100);
    REQUIRE(c3.g == 150);
    REQUIRE(c3.b == 200);
}

TEST_CASE("Color modifiers", "[color]") {
    Color c(100, 150, 200);

    SECTION("Bold") {
        Color bold = c.with_bold(true);
        REQUIRE(bold.bold == true);
        REQUIRE(bold.r == c.r);
    }

    SECTION("Italic") {
        Color italic = c.with_italic(true);
        REQUIRE(italic.italic == true);
    }

    SECTION("Underline") {
        Color underline = c.with_underline(true);
        REQUIRE(underline.underline == true);
    }

    SECTION("Combined") {
        Color styled = c.with_bold().with_italic().with_underline();
        REQUIRE(styled.bold == true);
        REQUIRE(styled.italic == true);
        REQUIRE(styled.underline == true);
    }
}

TEST_CASE("Color blend", "[color]") {
    Color c1(0, 0, 0);
    Color c2(100, 100, 100);

    Color blended = c1.blend(c2, 0.5f);
    REQUIRE(blended.r == 50);
    REQUIRE(blended.g == 50);
    REQUIRE(blended.b == 50);
}

TEST_CASE("Color darken/lighten", "[color]") {
    Color c(100, 100, 100);

    Color darker = c.darken(0.5f);
    REQUIRE(darker.r == 50);
    REQUIRE(darker.g == 50);
    REQUIRE(darker.b == 50);

    Color lighter = c.lighten(0.5f);
    REQUIRE(lighter.r > c.r);
    REQUIRE(lighter.g > c.g);
    REQUIRE(lighter.b > c.b);
}

TEST_CASE("Color is_dark", "[color]") {
    Color dark(50, 50, 50);
    REQUIRE(dark.is_dark() == true);

    Color light(200, 200, 200);
    REQUIRE(light.is_dark() == false);
}

TEST_CASE("Color contrast", "[color]") {
    Color dark(50, 50, 50);
    Color contrast_dark = dark.contrast();
    REQUIRE(contrast_dark == Color(255, 255, 255));

    Color light(200, 200, 200);
    Color contrast_light = light.contrast();
    REQUIRE(contrast_light == Color(0, 0, 0));
}

TEST_CASE("Color ANSI generation", "[color]") {
    Color c(255, 128, 0);

    std::string ansi_disabled = c.toAnsi(ColorMode::Disabled);
    REQUIRE(ansi_disabled == "");

    std::string ansi_16 = c.toAnsi(ColorMode::Ansi16);
    REQUIRE(ansi_16.find("\033[") == 0);
    REQUIRE(ansi_16.find("m") != std::string::npos);

    std::string ansi_256 = c.toAnsi(ColorMode::Ansi256);
    REQUIRE(ansi_256.find("\033[") == 0);
    REQUIRE(ansi_256.find("38;5;") != std::string::npos);

    std::string ansi_true = c.toAnsi(ColorMode::TrueColor);
    REQUIRE(ansi_true.find("\033[") == 0);
    REQUIRE(ansi_true.find("38;2;") != std::string::npos);
}

TEST_CASE("Color predefined colors", "[color]") {
    using namespace colors;
    REQUIRE(black == Color(0, 0, 0));
    REQUIRE(red == Color(170, 0, 0));
    REQUIRE(green == Color(0, 170, 0));
    REQUIRE(bright_white == Color(255, 255, 255));
}

TEST_CASE("Color equality", "[color]") {
    Color c1(100, 150, 200);
    Color c2(100, 150, 200);
    Color c3(100, 150, 201);

    REQUIRE(c1 == c2);
    REQUIRE(c1 != c3);
    REQUIRE_FALSE(c1 == c3);
}
