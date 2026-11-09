# PD2 BH with MapHack

## What is it
Project Diablo 2's version of BH with added MapHack capability, for singleplayer. Tested on S14 patch 2. 

## How to use it
Compile with Visual Studio, then replace BH.dll in ProjectD2 directory with compiled library. Press 9 to reveal current act or PD2 map you are currently on.

## How to compile it
Download the source code, open .sln file in Visual Studio. Run build. If successful, compiled library will be in /Debug (or wherever VS spits out the file, it also likes to copy those output files to the drive's top directory).

## Limitations
- map id 157 (Pvp Map; desert arena, from Akara) and 161 (Necropolis Jungle) can't be revealed as they crash the game for some reason on reveal
- shrines' and other objects' markers disappear from the map after leaving the game, do reveal again to restore them for the current game
- in some places (maps) shrines don't get revealed

## Can I use it online?
Probably not

## I'm updating/launching the game though the official launcher and map reveal doesn't work
The launcher has overwritten BH.dll to the official one, replace it again and don't use the launcher to launch the game (e.g. use Diablo II.exe).
