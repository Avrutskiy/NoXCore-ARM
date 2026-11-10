# NoXCore-Mac

Apple Silicon / Metal port of [NoX Core](https://github.com/mordentral/NoXCore).

The Mac implementation is isolated in `macos/`. Apart from this README, the fork adds
only Mac implementation and workflow files. Upstream fonts, gameplay, input mapping
and editor code are retained.

**[Download the Mac build](https://github.com/Avrutskiy/NoXCore-Mac/releases/latest).**

Mac-specific issues belong in [this fork's issue tracker](https://github.com/Avrutskiy/NoXCore-Mac/issues).
Upstream does not build, test or support macOS. See [macos/BUILDING.txt](macos/BUILDING.txt) for build and
maintenance details. Release assets are built from the tagged source revision.

**A modern client for Nox** (Westwood Studios, 2000): the game as you know it, cleaned up, with
new features, for today's Apple Silicon Macs.

NoX Core plays just like the original and speaks its network protocol, so NoX Core players and
players on the original game join each other's games, on the original's servers and on ours. Bugs
that cheaters and troublemakers could use are fixed; the quirks players have come to rely on are
kept. It is written from scratch, from our own analysis of how the game behaves; no code from the
original game is included.

- **Upstream website:** https://noxhub.net/
- **Everything it adds:** https://noxhub.net/features.html
- **What changed lately:** https://noxhub.net/patch-notes.html
- **Video walkthroughs** (map editor, scripting, mods): https://noxhub.net/walkthroughs.html

> You need your own copy of Nox (GOG, Origin or the original CD). NoX Core contains none of the
> game's art, sound, music, maps or text; it reads them from your installed game when it runs.

## Highlights

- **Co-op Campaign:** play the Warrior's, Wizard's or Conjurer's story with up to three friends,
  original-game players included.
- **New modes:** Delve (endless generated levels) and Undead Tide (survive the night), alone or
  with a party.
- **Map Editor** inside the game: walls, floors, objects, scripts with a reference and
  suggestions, play-testing with F5, and editing together with friends.
- **Hosting** from the menus with the original's full server settings, bots, map voting and
  faster updates for NoX Core players.
- **Graphics:** widescreen, any window size, per-pixel lighting, bloom, and an optional pack of
  HD images.
- **Mods:** game modes written in Luau, loaded from the `mods` folder.
- **The NoX Content Editor** for the game's files, saving changes into content packs.

## Building

Short version (full details, prerequisites and options in [macos/BUILDING.txt](macos/BUILDING.txt)):

**macOS ARM64** (macOS 26+, Xcode 26 Command Line Tools, CMake, Ninja, glslang and SPIRV-Cross):

```
cmake -S . -B build/macos-arm64 -G Ninja -C macos/cache.cmake
cmake --build build/macos-arm64 --target noxsdl_bundle -j 4
```

The app is `build/macos-arm64/NoX Core.app`, ad-hoc signed and not notarized.
Place it in your Nox data folder, or copy your data to
`~/Library/Application Support/NoXCore/GameData`. Settings and saves use
`~/Library/Application Support/NoXCore/UserData`. See [macos/BUILDING.txt](macos/BUILDING.txt) for details.

The opt-in notch/safe-area changes in upstream PR #23 are not included yet. Exclusive
fullscreen on a notched display has a known layout/input limitation; use windowed mode
if content or pointer alignment is affected.

The first configure downloads the pinned third-party libraries, so it needs a network connection.

## Running

Extract the release ZIP and open **NoX Core.app** in Finder. Place the app in your Nox data
folder, or copy the game's data to `~/Library/Application Support/NoXCore/GameData`.
Your own Nox game data is required; it is not included in the download.

To select a different data folder from Terminal, launch the app with arguments:

```sh
open "/path/to/NoX Core.app" --args --game-dir "/path/to/Nox"
```

The app opts into macOS Game Mode, which activates automatically in native full screen.
Launch from Finder or use `open` as above; running the executable directly from Terminal
may prevent activation. Game Mode does not activate in a regular window. If you previously
turned it off for this app, turn it back on in the macOS Game Overlay (Command–Esc).

### Saves, settings and backups

The Mac build keeps each user's progress and settings outside the app and game-data folder:

| Files | Default location |
|---|---|
| Characters and campaign saves | `~/Library/Application Support/NoXCore/UserData/Save` |
| Settings (`nox.cfg`, `noxcore.cfg`) | `~/Library/Application Support/NoXCore/UserData` |
| Game log | `~/Library/Application Support/noxcpp/noxsdl/noxcore-log.txt` |

In Finder, press **Shift–Command–G** (Go → Go to Folder) and paste a location above.
The `~` means your home folder. These directories are created when the game first runs.

To back up or transfer progress, quit the game and copy the **whole `Save` folder**, including
its subfolders. To restore it, quit the game, keep a copy of the current folder, and replace it
with your backup. Copy `nox.cfg` and `noxcore.cfg` too if you want to keep your settings.
Replacing **NoX Core.app** during an update leaves these files in place.

On first use, existing settings and saves from the selected game-data folder are copied into
the user directory without overwriting existing files. The originals remain in place.
Later launches do not repeat that import or restore characters you have deleted.

For a custom location, pass `--user-dir "/path/to/UserData"`. It changes the settings directory
and the default save location to its `Save` subfolder. An explicit
`--save-dir "/path/to/Save"` overrides the save location and skips automatic save import.

## What's in this repository

| Folder | What it holds |
|---|---|
| `macos/` | The isolated Mac implementation: startup, user data, Metal shaders, app packaging, build configuration and tests. |
| `noxcore/` | The engine library: the game's file formats, its network protocol, and the game simulation that hosted games run on. |
| `noxsdl/` | The NoX Core client on SDL3: menus, play, hosting, rendering, audio and the map editor. |
| `modes/` | The built-in game modes (Delve, Undead Tide, the co-op campaign) and the Luau scripting that mods use. |
| `mods/` | Mods written in Luau: Deep Delve, an action-RPG mode in development and the example to learn from. |
| `videotools/` | The NoX Content Editor (`noxcontent`). How to use it: `videotools/CONTENT_EDITOR.txt`. |
| `docs/original_bugs.md` | The bugs found in the original game, and which are fixed or kept. |
| `docs/vanilla_deviations.md` | Where NoX Core deliberately behaves differently from the original, and why. |
| `cmake/`, `.github/`, `packaging/` | Build settings and pinned dependencies, the automatic build, and a debug launcher. |

## Reporting bugs

Bug reports are welcome as GitHub issues. Please include
`~/Library/Application Support/noxcpp/noxsdl/noxcore-log.txt` and say whether the game was
hosted by NoX Core or by the original game.

## Licence

NoX Core is free software under the GNU General Public License, version 3 or (at your option) any
later version: see [LICENSE](LICENSE) and [NOTICE.txt](NOTICE.txt). The third-party code it uses,
and those licences, are listed in [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt).

NoX Core is an unofficial fan project, not affiliated with, endorsed by or sponsored by Electronic
Arts or Westwood Studios. Nox is a trademark of Electronic Arts Inc.
