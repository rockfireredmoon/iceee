# About IceEE - A Planet Forever Server

## Introduction
This is the Git repository for the server data and source for IceEE, the continuation of Grethnefar's Planetforever Server. Changes on the dev server will periodically be pushed to this, as will changes by developers and their local server. 
 
### Source Modifications
 
 I will try to keep an overview of changes to the server source it was released.
 
  * Allow scripts to use Tab (\t) characters so improve compatibility and readability of scripts.
  * Add SO_REUSEADDR socket option so the server can be started and stopped quickly without having to wait for sockets to timeout.
  * Added ability to choose address for server to bind to. This allows multiple servers to be run on the same host without making client modifications by multihoming the host (mulitple IP addresses). Each server has it's own ServerConfig.txt that specifies the address to bind to, instead of listening to all addresses on the host. 
  * Added new instance script command 'despawn' to remove spawns given their PropID
  * Added new ability action 'RemoveHealthBuff' that can remove BONUS_HEALTH buffers (and con and health)
  * Added 'Guilds', kind of like clans, but fixed groups in game, opening quests and other guild specific content.
  * Transformation potions
  * Full and working GM screen (/gm)
  * Server side Squirrel scripting
  * New script commands to spawns effects and props
  * Markers command to help developers warp quickly (/markers)
  * Zone definitions can specify environments for individual tiles or rectangular areas.
  * In-game script editor (/iscript)

### Credits

Many thanks to all those involved in making this game, and keeping it alive.

 * Grethnefar - Original programming. The Rebuilder.
 * Emerald Icemoon - Programming, content and admin
 * Heathendel Dustrunner - Quest, content and battle design and more.
 * Rictar Gasper - World building
 * Liska Quicksilver - Quest design and Lore Master
 * Rivers Slypaw - Item and armour design
 * And all the other IGF contributors
 
#### Audio

Credits for additional audio content added to the game.

#####Gloom Horizon - Ambient music used in Grunes Tal undead region

Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 3.0
http://creativecommons.org/licenses/by/3.0/

#####Kankbeeld Horror Pack - Ambient background used in Grunes Tal

Sound from http://www.freesound.org/people/klankbeeld/

#####Forest Evening - Ambient background now using in Grunes Tal

Sound from https://www.freesound.org/people/cediez/

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