# About TAWD - An Earth Eternal Server

## Introduction

This is the Git repository for the server data and source for TAWD, the continuation of the IceEE project (SOAD, VALD), which itself is a continuation Grethnefar's Planetforever Server. Changes on the dev server will periodically be pushed to this, as will changes by developers and their local server. 
 
### Source Modifications
 
 In addition to the large number of changes in the earlier IceEE servers, TAWD is an attempt to address
 some architectural problems, add new mechanics and web site integration and more.
 
  * HTTP server component replaced with CivetWeb, an embeddable C/C++ HTTP server. This brings HTTPS support
    and other performance, scalibity, security and maintenance advantages. 
  * Replaced logging system with easylogging
  * New quest mechanics (ad-hoc quests, 'outcomes')
  * Abstracted message / query system
  * Clan support
  * HTTP API for website integration
  * Other tidying

### Credits

Many thanks to all those involved in making this game, and keeping it alive.

 * Grethnefar - Original programming. The Rebuilder.
 * Emerald Icemoon - Programming, content and admin
 * Heathendel Dustrunner - Quest, content and battle design and more.
 * Rictar Gasper - World building
 * Liska Quicksilver - Quest design and Lore Master
 * And all the other IGF contributors
 
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
#####The Hive - Background Music used in Skrill Queen Lair
#####Constance - Fight Music used in Skrill Queen Lair
#####Christmas Rap - Ambient music used for Winter Dawning region
#####Wish Backgroud  - Ambient music used for Winter Dawning region

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
#####10 Second Countdown - thomasevd - https://freesound.org/people/thomasevd/sounds/202193/

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
options to ./configure. For example, to enable debugging symbols (recommended), use :-

```bash
./configure --enable-debug
```

#### Compile

All that remains now is to compile the source. The '-j 4' makes the compiler use up to 4 cores for compilation. 
This greatly speeds up the build. Adjust depending on the nuumber of cores you have (or want to use of course).

```bash
make -j4
```

### Configuring The Server

