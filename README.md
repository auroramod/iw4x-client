![license](https://img.shields.io/github/license/iw4x/iw4x-client.svg)
[![build](https://github.com/iw4x/iw4x-client/actions/workflows/build.yml/badge.svg?branch=develop)](https://github.com/iw4x/iw4x-client/actions)

# IW4x: Client

<img src=".github/assets/readme/icon.png" align="right" width="100" height="100">

This fork of IW4x is maintained by Aurora to give better modding capatibilties and be unrestricted from the main repository. This fork is always pulling in upstream updates, meaning you can enjoy the new features added here with the latest features from IW4x!

To learn more about IW4x itself, [check the original IW4x repository out.](https://github.com/iw4x/iw4x-client)

## Compiling from Source

To learn how to compile, [check out IW4x's Compiling from Source guide.](https://github.com/iw4x/iw4x-client?tab=readme-ov-file#compiling-from-source)

## New features with Aurora

- Re-added `reloadmenus` command (it is technically unstable, but is made accessible to modders for convinence of menu modding)
- Check if zone exists before running `loadzone`
- Add `take` command
- Add `give ammo` command, which fills up all weapons

## Command line arguments

| Argument                | Description                                    |
|:------------------------|:-----------------------------------------------|
| `-entries`              | Print to the console a list of every asset as they are loaded from zonefiles. |
| `-stdout`               | Redirect all logging output to the terminal iw4x is started from, or if there is none, creates a new terminal window to write log information in. |
| `-console`              | Allow the game to display its own separate interactive console window. |
| `-dedicated`            | Starts the game as a headless dedicated server. |
| `-bigminidumps`         | Include all code sections from loaded modules in the dump. |
| `-reallybigminidumps`   | Include data sections from all loaded modules in the dump. |
| `-dump`                 | Write info of loaded assets to the raw folder as they are being loaded. |
| `-nointro`              | Skip game's cinematic intro.                   |
| `-version`              | Print IW4x build info on startup.              |
| `-steam`                | Enable friends feature and other Steam integrations. |
| `-unprotect-dvars`      | Allow the server to modify saved/archive dvars. |
| `-zonebuilder`          | Start the interactive zonebuilder tool console instead of starting the game. |
| `-original-str-parsing` | (ZoneBuilder mode only) Parse .str files in the same manner as the CoD4 Mod Tools. |
| `-disable-notifies`     | Disable "Anti-CFG" checks |
| `-disable-mongoose`     | Disable Mongoose HTTP server |
| `-disable-rate-limit-check` | Disable RCon rate limit checks |
| `-disable-mod-unloading` | Disable automatic mod (fs_game) unloading when disconnecting |

## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.
