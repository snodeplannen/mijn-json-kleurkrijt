#!/usr/bin/env python3
"""Demonstratie van individuele kleuren per key/veld"""
import colored_json

# Voorbeeld data
data = {
    "user": {
        "name": "Alice",
        "age": 30,
        "email": "alice@example.com",
        "active": True
    },
    "products": [
        {"id": 1, "name": "Laptop", "price": 999.99},
        {"id": 2, "name": "Muis", "price": 29.99}
    ],
    "status": "success",
    "count": 42
}

print("=" * 60)
print("INDIVIDUELE KLEUREN PER KEY/VELD DEMONSTRATIE")
print("=" * 60)

# Basis style
style = colored_json.Style.get_preset("default")
style.compact = False

print("\n=== Standaard (zonder individuele kleuren) ===")
colored_json.print(data, style)

print("\n=== Individuele key kleuren ===")
# Specifieke keys krijgen een andere kleur
style.set_key_color("user", colored_json.Color(255, 0, 0))  # Rood voor "user" key
style.set_key_color("name", colored_json.Color(0, 255, 0))  # Groen voor "name" keys
style.set_key_color("price", colored_json.Color(255, 165, 0))  # Oranje voor "price" keys
style.set_key_color("status", colored_json.colors.bright_cyan)  # Cyaan voor "status" key

colored_json.print(data, style)

print("\n=== Individuele value kleuren (op basis van key path) ===")
# Reset key colors
style.key_colors.clear()

# Values krijgen een andere kleur op basis van hun pad
style.set_value_color("user.name", colored_json.Color(255, 20, 147))  # Dieproze voor user.name waarde
style.set_value_color("user.age", colored_json.Color(0, 191, 255))  # Diepblauw voor user.age waarde
style.set_value_color("user.email", colored_json.Color(255, 140, 0))  # Donkeroranje voor user.email waarde
style.set_value_color("status", colored_json.colors.bright_green)  # Groen voor status waarde
style.set_value_color("count", colored_json.Color(255, 215, 0))  # Goud voor count waarde

colored_json.print(data, style)

print("\n=== Combinatie: key EN value kleuren ===")
# Beide kunnen gecombineerd worden
style.set_key_color("user", colored_json.Color(255, 0, 0))  # Rode key
style.set_key_color("products", colored_json.Color(138, 43, 226))  # Paarse key
style.set_value_color("user.name", colored_json.Color(255, 20, 147))  # Dieproze value
style.set_value_color("user.age", colored_json.Color(0, 191, 255))  # Diepblauw value

colored_json.print(data, style)

print("\n=== Geneste paths ===")
# Voor geneste structuren kun je het volledige pad gebruiken
style.key_colors.clear()
style.value_colors.clear()

style.set_value_color("user.name", colored_json.colors.bright_yellow)
style.set_value_color("user.email", colored_json.colors.bright_cyan)
style.set_value_color("products[0].name", colored_json.colors.bright_green)
style.set_value_color("products[1].name", colored_json.colors.bright_magenta)
style.set_value_color("products[0].price", colored_json.Color(255, 215, 0))  # Goud
style.set_value_color("products[1].price", colored_json.Color(255, 140, 0))  # Donkeroranje

colored_json.print(data, style)

print("\n=== Met bold/italic styling ===")
style.key_colors.clear()
style.value_colors.clear()

# Key met bold
user_key_color = colored_json.Color(255, 0, 0)
user_key_color.bold = True
style.set_key_color("user", user_key_color)

# Value met italic (via custom color)
custom_color = colored_json.Color(0, 191, 255)
custom_color.italic = True
style.set_value_color("user.name", custom_color)

colored_json.print(data, style)

print("\n" + "=" * 60)
print("DEMONSTRATIE VOLTOOID")
print("=" * 60)

