# PD2 BH with MapHack

## What is it
Project Diablo 2's version of BH with added MapHack capability. Tested on S11 patch 2. I didn't test it on other patches and don't plan to.

## How
I've added map reveal code from original slashdiablo's BH and made it run on keypress in MapNotify module.

## How to use it
Compile it yourself with Visual Studio, then replace BH.dll in ProjectD2 directory with compiled library. Press 9 to reveal current act.

## How to compile it
Download source code, open .sln in Visual Studio. Run build. If successful, compiled library will be in /Debug (or wherever VS spits out the file).

## Limitations
- to reveal maps on Hell, zone into one and press reveal, this takes longer than act reveal as maps are more complex
- if you try to reveal maps on difficulties other than Hell, only the map you're currently on will be revealed (to my knowledge this only applies to pvp maps)
- map id 157 (Pvp Map; desert arena, from Akara) and 161 (Necropolis Jungle) can't be revealed as they crash the game for some reason on reveal
- shrines' and other objects' markers disappear from the map after leaving the game, do reveal again to restore them for the current game

## Can I use it in singleplayer?
Yes that's the point

## Can I use it online?
I don't (and don't plan to) play online so I don't know; from what I've heard you won't be able to connect due to hash mismatch of the dll (or the dll will be overwritten with the offical one). I've also put zero effort in making this undetectable, so you'll probably be banned very quickly.

## Can you update it so that it works on Season X Patch Y?
No, unless I also plan on playing it. Do it yourself otherwise.
