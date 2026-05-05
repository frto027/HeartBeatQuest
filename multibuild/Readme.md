# What's Inside This Folder

This folder contains different QPM manifests for multiple game versions.

The `target.cmake` file is included in `CMakeLists.txt` for code compatibility.

The main idea is to release multiple `.qmod` files from the same source code, so older game versions can still receive the latest updates.

# Version Switching

Each `.json` file describes replacement rules for the corresponding QPM manifest. Other files are replaced directly.

`qpm.shared.json` is not tracked in git — QPM decides which version to use.

Available C++ macros:
- Game-version macros, e.g. `GAME_VER_1_35_0`
- A string macro called `GAME_VERSION`, e.g. `"1_35_0"`

# Auto-Update Mod Versions

This folder also contains scripts that automatically update mod dependencies when the game receives small changes that don't affect this mod.
