# GamePort

A terminal UI for discovering, installing, and launching open-source games.

- 50+ open-source games across 12 categories
- One-key install and launch
- Filter by platform (Linux, macOS, Windows)
- Search, browse by category, view controls
- Tracks installed games, shows disk usage

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

<details>
<summary><b>Action</b> (5)</summary>
Tetris, RecWars, ClassiCube, Armagetron Advanced, Luanti
</details>

<details>
<summary><b>Card</b> (1)</summary>
PokerTH
</details>

<details>
<summary><b>Platformer</b> (2)</summary>
Sonic Robo Blast 2, SuperTux
</details>

<details>
<summary><b>Puzzle</b> (3)</summary>
Minesweeper, 2048, Colobot
</details>

<details>
<summary><b>Racing</b> (2)</summary>
SuperTuxKart, Speed Dreams
</details>

<details>
<summary><b>Roguelike</b> (5)</summary>
Brogue CE, Cataclysm: DDA, NetHack, Dungeon Crawl Stone Soup, Angband
</details>

<details>
<summary><b>RPG</b> (3)</summary>
Flare RPG, Veloren, The Ur-Quan Masters
</details>

<details>
<summary><b>Shooter</b> (8)</summary>
Space Invaders, Asteroids, Anarch, Chocolate Doom, Red Eclipse, C-Dogs SDL, Teeworlds, Xonotic
</details>

<details>
<summary><b>Simulation</b> (4)</summary>
OpenTTD, Endless Sky, Naev, Pioneer
</details>

<details>
<summary><b>Stealth</b> (1)</summary>
The Dark Mod
</details>

<details>
<summary><b>Strategy</b> (13)</summary>
Chess TUI, Warzone 2100, Widelands, Battle for Wesnoth, MegaGlest, 0 A.D., OpenRA: Red Alert, Unciv, FreeCiv, FreeOrion, Hedgewars, Mindustry, OpenClonk
</details>

<details>
<summary><b>Typing</b> (1)</summary>
ttyper
</details>

## License

MIT
