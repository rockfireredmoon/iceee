# About VALD - A Server for Earth Eternal - Valkal's Shadow

## Introduction
This is the Git repository for the server data and source for Earth Eternal - Valkal's Shadow, the continuation of Grethnefar's "Planet Forever" Server. Changes on the developer server will periodically be pushed to this, as will changes by developers and their local server. 

Find out much more at our website.

http://www.theanubianwar.com
 
### Changes 
 
This is a very rough list of some of the larger changes made since the official "Earth Eternal" and the subsequent
"Planet Forever"
   
  * Transformation potions
  * Full and working GM screen (/gm)
  * Server side Squirrel scripting and a much larger API for making quests and dungeons more interesting
  * Server-scripted animation. Simple animations such as moving, scaling, rotating and fading may be performed 
    on props. 
  * Markers command to help developers warp quickly (/markers)
  * Zone definitions can specify environments for individual tiles or rectangular areas.
  * In-game script editor (/iscript)
  * "Books" for delivering lore and other non-critical information 
  * Dungeons can have day cycles (separate from overworld or linked)
  * Two new regions
  * New audio content
  * Daily rewards
  * NPC dialog
  * New special effects, abilities and ability functions
  * Credits now active
  * Credit Shop now active including new player made armour sets.
  * Ability to buy new groves
  * Ability to buy character slots (up to 8)
  * Temporary buffs
  * Persistent buffs
  * Improved registration system
  * Lots of bug fixes and minor changes

### Credits

Many thanks to all those involved in making this game, and keeping it alive.

 * Grethnefar - Original programming. The Rebuilder.
 * Emerald Icemoon - Programming, content and admin
 * Heathendel Dustrunner - Quest, content and battle design and more.
 * Rictar Gasper - World building
 * Liska Quicksilver - Quest design and Lore Master
 * And all the other contributors, players and community members
 
#### Audio

Credits for additional audio content added to the game.

#####Gloom Horizon - Ambient music used in Grunes Tal undead region
#####Crossing The Chasm - Ambient music used in Swineland region
#####Dark Times - Activate music used in Bloodkeep
#####Land Of Phantoms - Ambient music used in Bloodkeep
#####Some Amount Of Evil - Ambient music used in Bloodkeep
#####Killers - Ambient music used in penultimate fight scene in Bloodkeep
#####Chee Zee Cave - Music used in Southend Passage
#####Night Cave - Music used in Forest Cave
#####Curse Of The Scarab - Music used in Djinn Temple
#####Dragon And Toast - Music used in Fangarian's Lair
#####Grim Idol - Fight Music used in Fangarian's Lair

Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 3.0
http://creativecommons.org/licenses/by/3.0/

#####Tap - Finale in Valkal's Bloodkeep
Music: Alexander Nakarada (www.serpentsoundstudios.com)
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/

Lots of sound effects used as background or spot effects throughout the
game came from freesound.org. All of these are 
Licensed under Creative Commons: By Attribution 3.0

#####Forest Evening - cediez - https://www.freesound.org/people/cediez/ 
#####Kankbeeld Horror Pack - Kankbeeld - http://www.freesound.org/people/klankbeeld/
#####Dungeon Ambiance - https://freesound.org/people/phlair/sounds/388340/
#####Lava loop - Audionautics - https://freesound.org/people/Audionautics/sounds/133901/
#####Dripping Cave - dobroide - https://freesound.org/people/dobroide/sounds/396314/
#####Thunder Pack - hantorio - https://freesound.org/people/hantorio/packs/7640/ 

 And some from https://tabletopaudio.com
 Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.

#####In The Shadows - Ambient background 

## Configuring A Server From This Repository

This a subset of the data required for a full working server, it consists of the server source, and most of the static data (except notably assets).

In order to run a server, the basic steps are :-

1. Compile the server source
2. Configure the server and create required files.
3. Start the server daemon.

### Compiling The Server

This project is distributed as an 'autotools' project. 

#### Preparing the source tree

Before compilation of the source tree can take place, the raw source tree from GitHub must
be processed, creating additional files to aid configuration of the source appropriate for
your platform.

If you have downloaded a source bundle, this step is not required so you can jump straight
to Configure The Source.  

You will need the following packages installed :-

* autoconf
* automake
* libtool
* g++

Other packages may be needed for various common libraries used. All of these should be
easily obtainable for your OS/Distribution. 
 
Now run :-

```bash
autoreconf -fi
```

#### Configuration of the source tree

Now all of the autotools files are in place, you can configure the tree for compilation on
your platform.

Run :-

```bash
./configure  
```

The configure script will examine your system for the required libraries and tools and produce a report 
as to which compile time options have been selected. You can alter the behavior of the build by passing
options to ./configure. For example, to enable debuggin symbols (recommended), use :-

```bash
./configure --enable-debug
```

#### Compile

All that remains now is to compile the source. The '-j 4' makes the compiler use up to 4 cores for compilation. 
This greatly speeds up the build. Adjust depending on the nuumber of cores you have (or want to use of course).

```bash
make -j4
```