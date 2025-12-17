# PD2 BH with MapHack

## What is it
Project Diablo 2's version of BH with added MapHack capability. Tested on S12 patch 2. I didn't test it on other patches and don't plan to.

## How
I've added map reveal code from original slashdiablo's BH and made it run on keypress in MapNotify module.

## How to use it
Compile it yourself with Visual Studio, then replace BH.dll in ProjectD2 directory with compiled library. Press 9 to reveal current act or PD2 map you are currently on.

## How to compile it
Download source code, open .sln in Visual Studio. Run build. If successful, compiled library will be in /Debug (or wherever VS spits out the file).

## Limitations
- map id 157 (Pvp Map; desert arena, from Akara) and 161 (Necropolis Jungle) can't be revealed as they crash the game for some reason on reveal
- shrines' and other objects' markers disappear from the map after leaving the game, do reveal again to restore them for the current game
- in some places (maps) shrines don't get revealed

## Can I use it in singleplayer?
Yes that's the point

## Can I use it online?
I don't (and don't plan to) play online so I don't know; from what I've heard you won't be able to connect due to hash mismatch of the dll. I've also put zero effort in making this undetectable, so you'll probably be banned very quickly.

## Can you update it so that it works on Season X Patch Y?
No, unless I also plan on playing it. Do it yourself otherwise.

## I'm updating/launching the game though the official launcher and map reveal doesn't work
The launcher has overwritten BH.dll to the official one, replace it again and don't use the launcher to launch the game (e.g. use Diablo II.exe).
