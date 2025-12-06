"""
Type stubs voor colored_json module.

Deze file bevat type hints voor alle classes, functies en enums
die geëxposeerd worden via pybind11.
"""

from typing import Dict, Any, List, Union

# Enums
class ColorMode:
    """Kleur modus voor ANSI escape codes."""
    AUTO: ColorMode
    ANSI16: ColorMode
    ANSI256: ColorMode
    TRUECOLOR: ColorMode
    DISABLED: ColorMode

class ElementType:
    """Type elementen in JSON structuur."""
    KEY: ElementType
    STRING: ElementType
    NUMBER: ElementType
    BOOLEAN: ElementType
    NULL: ElementType
    BRACE: ElementType
    BRACKET: ElementType
    COLON: ElementType
    COMMA: ElementType
    WHITESPACE: ElementType

# Classes
class Color:
    """RGB kleur met optionele tekst styling."""
    
    r: int  # 0-255
    g: int  # 0-255
    b: int  # 0-255
    bold: bool
    italic: bool
    underline: bool
    
    def __init__(self, r: int, g: int, b: int) -> None:
        """Maak een nieuwe Color aan.
        
        Args:
            r: Rood component (0-255)
            g: Groen component (0-255)
            b: Blauw component (0-255)
        """
        ...
    
    def to_ansi(self, mode: ColorMode = ColorMode.AUTO) -> str:
        """Converteer naar ANSI escape code string.
        
        Args:
            mode: ColorMode voor conversie (standaard: AUTO)
        
        Returns:
            ANSI escape code string
        """
        ...
    
    @staticmethod
    def from_rgb(r: int, g: int, b: int) -> "Color":
        """Maak Color van RGB waarden (factory method).
        
        Args:
            r: Rood component (0-255)
            g: Groen component (0-255)
            b: Blauw component (0-255)
        
        Returns:
            Nieuwe Color instance
        """
        ...

class Style:
    """Stijl configuratie voor kleuring en formatting van JSON."""
    
    key_color: Color
    string_color: Color
    number_color: Color
    bool_color: Color
    null_color: Color
    brace_color: Color
    bracket_color: Color
    colon_color: Color
    comma_color: Color
    key_quote_color: Color
    string_quote_color: Color
    key_colors: Dict[str, Color]
    value_colors: Dict[str, Color]
    color_mode: ColorMode
    compact: bool
    indent_size: int
    
    def __init__(self) -> None:
        """Maak een nieuwe Style met standaard kleuren."""
        ...
    
    def set_key_color(self, key: str, color: Color) -> None:
        """Stel individuele kleur in voor een specifieke key.
        
        Args:
            key: De key naam (bijv. "user", "name", "status")
            color: Color object voor deze key
        """
        ...
    
    def set_value_color(self, path: str, color: Color) -> None:
        """Stel individuele kleur in voor een value op basis van pad.
        
        Args:
            path: Pad naar de value (bijv. "user.name", "products[0].price")
            color: Color object voor deze value
        """
        ...
    
    @staticmethod
    def get_preset(name: str) -> "Style":
        """Laad een preset thema.
        
        Beschikbare presets: default, dracula, solarized, monokai,
        github, minimal, neon
        
        Args:
            name: Naam van de preset
        
        Returns:
            Style object met preset configuratie
        
        Raises:
            RuntimeError: Als preset naam niet bestaat
        """
        ...
    
    @staticmethod
    def list_presets() -> List[str]:
        """Lijst van beschikbare preset thema's.
        
        Returns:
            Lijst van preset namen
        """
        ...

class Printer:
    """Printer class voor console output met ANSI kleuren."""
    
    def __init__(self, style: Style) -> None:
        """Maak een nieuwe Printer instance.
        
        Args:
            style: Style object voor kleuring en formatting
        """
        ...
    
    def print(self, obj: Union[str, Any]) -> str:
        """Formatteer en print een Python object of JSON string.
        
        Ondersteunt zowel Python objecten (dict, list) als JSON strings.
        
        Args:
            obj: Python dict, list, JSON string, of ander object om te formatteren
        
        Returns:
            Gekleurde string met ANSI escape codes
        """
        ...

class HtmlPrinter:
    """Printer class voor HTML output generatie."""
    
    def __init__(self, style: Style) -> None:
        """Maak een nieuwe HtmlPrinter instance.
        
        Args:
            style: Style object voor kleuring en formatting
        """
        ...
    
    def print(
        self,
        obj: Union[str, Any],
        title: str = "Colored JSON",
        background_color: str = "#1e1e1e",
        font_family: str = "Consolas, 'Courier New', monospace"
    ) -> str:
        """Genereer complete HTML pagina met gekleurde JSON.
        
        Ondersteunt zowel Python objecten (dict, list) als JSON strings.
        
        Args:
            obj: Python dict, list, JSON string, of ander object om te formatteren
            title: Titel voor de HTML pagina
            background_color: Achtergrondkleur (CSS kleur string)
            font_family: Font familie voor de JSON weergave
        
        Returns:
            Complete HTML string met inline styles
        """
        ...

class MarkdownPrinter:
    """Printer class voor Markdown output generatie."""
    
    def __init__(self, style: Style) -> None:
        """Maak een nieuwe MarkdownPrinter instance.
        
        Args:
            style: Style object voor kleuring en formatting
        """
        ...
    
    def print(
        self,
        obj: Union[str, Any],
        title: str = "Colored JSON",
        language: str = "json"
    ) -> str:
        """Genereer Markdown met code block (zonder kleuren).
        
        Ondersteunt zowel Python objecten (dict, list) als JSON strings.
        
        Args:
            obj: Python dict, list, JSON string, of ander object om te formatteren
            title: Titel voor de Markdown sectie (optioneel)
            language: Code block language identifier (standaard: "json")
        
        Returns:
            Markdown string met code block
        """
        ...
    
    def print_html(
        self,
        obj: Union[str, Any],
        title: str = "Colored JSON",
        background_color: str = "#1e1e1e",
        font_family: str = "Consolas, 'Courier New', monospace"
    ) -> str:
        """Genereer Markdown met HTML code block voor gekleurde JSON.
        
        Ondersteunt zowel Python objecten (dict, list) als JSON strings.
        
        Args:
            obj: Python dict, list, JSON string, of ander object om te formatteren
            title: Titel voor de Markdown sectie (optioneel)
            background_color: Achtergrondkleur (CSS kleur string)
            font_family: Font familie voor de JSON weergave
        
        Returns:
            Markdown string met HTML code block
        """
        ...

# Convenience functies
def print(obj: Any, style: Style = ...) -> None:
    """Print een Python object direct naar de console met kleuren.
    
    Let op: Deze functie gebruikt py::print() en vereist de GIL.
    Voor JSON string input, gebruik format() of format_from_json() in plaats daarvan.
    
    Args:
        obj: Python dict, list, of ander object om te printen
        style: Optionele Style object (standaard: Style())
    """
    ...

def format(obj: Union[str, Any], style: Style = ...) -> str:
    """Formatteer een Python object of JSON string als gekleurde string.
    
    Ondersteunt zowel Python objecten (dict, list) als JSON strings.
    JSON strings worden automatisch gedetecteerd en geoptimaliseerd verwerkt.
    
    Args:
        obj: Python dict, list, JSON string, of ander object om te formatteren
        style: Optionele Style object (standaard: Style())
    
    Returns:
        Gekleurde string met ANSI escape codes
    """
    ...

def format_from_json(json_str: str, style: Style = ...) -> str:
    """Formatteer een JSON string als gekleurde string (expliciete functie).
    
    Geoptimaliseerd voor JSON string input met simdjson en GIL release.
    
    Args:
        json_str: JSON string om te formatteren
        style: Optionele Style object (standaard: Style())
    
    Returns:
        Gekleurde string met ANSI escape codes
    """
    ...

def to_html(
    obj: Union[str, Any],
    style: Style = ...,
    title: str = "Colored JSON",
    background_color: str = "#1e1e1e",
    font_family: str = "Consolas, 'Courier New', monospace"
) -> str:
    """Genereer HTML output van gekleurde JSON (convenience functie).
    
    Ondersteunt zowel Python objecten (dict, list) als JSON strings.
    
    Args:
        obj: Python dict, list, JSON string, of ander object om te formatteren
        style: Optionele Style object (standaard: Style())
        title: Titel voor de HTML pagina
        background_color: Achtergrondkleur (CSS kleur string, bijv. "#1e1e1e")
        font_family: Font familie voor de JSON weergave
    
    Returns:
        Complete HTML string met inline styles
    """
    ...

def to_html_from_json(
    json_str: str,
    style: Style = ...,
    title: str = "Colored JSON",
    background_color: str = "#1e1e1e",
    font_family: str = "Consolas, 'Courier New', monospace"
) -> str:
    """Genereer HTML output van JSON string (expliciete functie).
    
    Geoptimaliseerd voor JSON string input met simdjson en GIL release.
    
    Args:
        json_str: JSON string om te formatteren
        style: Optionele Style object (standaard: Style())
        title: Titel voor de HTML pagina
        background_color: Achtergrondkleur (CSS kleur string)
        font_family: Font familie voor de JSON weergave
    
    Returns:
        Complete HTML string met inline styles
    """
    ...

def to_markdown(
    obj: Union[str, Any],
    style: Style = ...,
    title: str = "Colored JSON",
    language: str = "json"
) -> str:
    """Genereer Markdown output met code block (convenience functie).
    
    Ondersteunt zowel Python objecten (dict, list) als JSON strings.
    
    Args:
        obj: Python dict, list, JSON string, of ander object om te formatteren
        style: Optionele Style object (standaard: Style())
        title: Titel voor de Markdown sectie
        language: Code block language identifier (standaard: "json")
    
    Returns:
        Markdown string met code block
    """
    ...

def to_markdown_from_json(
    json_str: str,
    style: Style = ...,
    title: str = "Colored JSON",
    language: str = "json"
) -> str:
    """Genereer Markdown output van JSON string (expliciete functie).
    
    Geoptimaliseerd voor JSON string input met simdjson en GIL release.
    
    Args:
        json_str: JSON string om te formatteren
        style: Optionele Style object (standaard: Style())
        title: Titel voor de Markdown sectie
        language: Code block language identifier (standaard: "json")
    
    Returns:
        Markdown string met code block
    """
    ...

def to_markdown_html(
    obj: Union[str, Any],
    style: Style = ...,
    title: str = "Colored JSON",
    background_color: str = "#1e1e1e",
    font_family: str = "Consolas, 'Courier New', monospace"
) -> str:
    """Genereer Markdown met HTML code block voor gekleurde JSON (convenience functie).
    
    Ondersteunt zowel Python objecten (dict, list) als JSON strings.
    
    Args:
        obj: Python dict, list, JSON string, of ander object om te formatteren
        style: Optionele Style object (standaard: Style())
        title: Titel voor de Markdown sectie
        background_color: Achtergrondkleur (CSS kleur string, bijv. "#1e1e1e")
        font_family: Font familie voor de JSON weergave
    
    Returns:
        Markdown string met HTML code block
    """
    ...

def to_markdown_html_from_json(
    json_str: str,
    style: Style = ...,
    title: str = "Colored JSON",
    background_color: str = "#1e1e1e",
    font_family: str = "Consolas, 'Courier New', monospace"
) -> str:
    """Genereer Markdown met HTML code block van JSON string (expliciete functie).
    
    Geoptimaliseerd voor JSON string input met simdjson en GIL release.
    
    Args:
        json_str: JSON string om te formatteren
        style: Optionele Style object (standaard: Style())
        title: Titel voor de Markdown sectie
        background_color: Achtergrondkleur (CSS kleur string)
        font_family: Font familie voor de JSON weergave
    
    Returns:
        Markdown string met HTML code block
    """
    ...

# Colors submodule
class colors:
    """Ingebouwde ANSI16 kleuren."""
    
    black: Color
    red: Color
    green: Color
    yellow: Color
    blue: Color
    magenta: Color
    cyan: Color
    white: Color
    bright_black: Color
    bright_red: Color
    bright_green: Color
    bright_yellow: Color
    bright_blue: Color
    bright_magenta: Color
    bright_cyan: Color
    bright_white: Color

