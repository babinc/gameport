# GamePort

A terminal UI for discovering, installing, and launching open-source games.

Browse 50+ games across 12 categories, install with one keypress, and playall without leaving the terminal.

![GamePort screenshot](https://raw.githubusercontent.com/babinc/gameport/master/screenshot.png)

## Install

### Prebuilt binaries

Download the latest release for your platform from [Releases](https://github.com/babinc/gameport/releases):

- `gameport-linux-x86_64.tar.gz`
- `gameport-macos-arm64.tar.gz`
- `gameport-windows-x86_64.zip`

Extract and run `gameport`.

### From source

Requires CMake 3.14+ and a C99 compiler.

```bash
git clone https://github.com/babinc/gameport.git
cd gameport
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/gameport
```

### Optional dependencies

Games are installed on demand. Depending on the game, you may need:

- **curl** downloads pre-built binaries and archives
- **cargo** installs Rust-based games via `cargo install`
- **git + cmake + make** builds games from source

GamePort shows which tools you have as badges in the header.

## Usage

| Key | Action |
|-----|--------|
| `j` / `k` | Navigate game list |
| `h` / `l` | Switch category |
| `p` | Cycle platform filter |
| `/` | Search games |
| `Enter` | Play selected game |
| `i` | Install selected game |
| `d` | Remove selected game |
| `c` | View game controls |
| `w` | Open website |
| `q` | Quit |

## Games

52 games across 12 categories:

| Category | Games |
|----------|-------|
| Action | Tetris, RecWars, ClassiCube, Armagetron Advanced, Luanti |
| Card | PokerTH |
| Platformer | Sonic Robo Blast 2, SuperTux |
| Puzzle | Minesweeper, 2048, Colobot |
| Racing | SuperTuxKart, Speed Dreams |
| Roguelike | Brogue CE, Cataclysm: DDA, NetHack, Dungeon Crawl Stone Soup, Angband |
| RPG | Flare RPG, Veloren, The Ur-Quan Masters |
| Shooter | Space Invaders, Asteroids, Anarch, Chocolate Doom, Red Eclipse, C-Dogs SDL, Teeworlds, Xonotic |
| Simulation | OpenTTD, Endless Sky, Naev, Pioneer |
| Stealth | The Dark Mod |
| Strategy | Chess TUI, Warzone 2100, Widelands, Battle for Wesnoth, MegaGlest, 0 A.D., OpenRA: Red Alert, Unciv, FreeCiv, FreeOrion, Hedgewars, Mindustry, OpenClonk |
| Typing | ttyper |

## Platforms

GamePort runs on Linux, macOS, and Windows. Each game lists which platforms it supports, and the platform filter lets you see only games available on your OS.

## License

MIT
