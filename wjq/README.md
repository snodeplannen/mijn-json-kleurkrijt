# wjq - Windows JSON Query Tool

Een krachtige, gekleurde JSON processor voor Windows met uitgebreide commandline opties.

## Installatie

### MinGW64 (MSYS2)
```bash
# Vereisten: cmake, gcc
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### Visual Studio
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Of open de folder in Visual Studio en gebruik de CMake integratie.

## Gebruik

Het programma wordt gebouwd in `build/Release/wjq.exe` (voor Visual Studio) of `build/wjq.exe` (MinGW).

### Basis commando's
```bash
# Lees bestand
wjq data.json

# Lees van stdin
cat data.json | wjq

# Specifiek thema
wjq -t monokai data.json

# Compacte output
wjq -c data.json

# Aangepaste indentatie
wjq -i 4 data.json

# Kleurmodus afdwingen
wjq --color-mode 256 data.json
```

### Commandline Opties
```
-t, --theme THEMA     Kies kleurenschema:
                      default, dracula, solarized, monokai, github, minimal, neon

-m, --color-mode MODE Kleurmodus:
                      auto (autodetect), 16, 256, truecolor, disabled

-c, --compact         Compacte output (minimale spaties/lijntjes)

-i, --indent N        Indentatiegrootte (1-8, standaard: 2)

-h, --help            Help weergeven

-v, --version         Versie weergeven
```

### Voorbeelden
```bash
# Neon thema met 256-kleuren mode
wjq -t neon -m 256 data.json

# GitHub thema, compact, indent van 4
wjq -t github -c -i 4 data.json

# Minimalistisch thema, geen kleur
wjq -t minimal --color-mode disabled data.json
```

## Kleurenschema's

| Thema      | Beschrijving                 |
|------------|------------------------------|
| default    | Dracula-achtig donker theme  |
| dracula    | Dracula theme                |
| solarized  | Solarized kleuren            |
| monokai    | Monokai theme                |
| github     | GitHub light theme (donkere versie) |
| minimal    | Subtiele grijswaarden        |
| neon       | Neon kleuren met bold text   |

### Kleurmodi
- **auto**: Autodetectie (aanbevolen)
- **truecolor**: 16.7 miljoen kleuren (beste kwaliteit)
- **256**: 256 kleuren (goede compatibiliteit)
- **16**: 16 basiskleuren (universeel)
- **disabled**: Geen kleuren (voor piped output)
