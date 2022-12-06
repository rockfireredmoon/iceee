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

### Configuring The Server

Before starting the server, you will probably want to configure it for your environment.

#### Configuring Server Behaviour

All other server behaviour is configured using the ServerConfig.txt file. As with all other configuration files, this is shipped as a ServerConfig.txt.sample, and must be copied and edited before
use.

```
cp ServerConfig.txt.sample ServerConfig.txt
```

Then edit *ServerConfig.txt* with your favourite text editor. This file contains many options,
most of which are commented. For a new installation, the ones that most likely need to be changed are :-

```
; The address which is advertised to clients via the 'router' service.  This address
; must be resolvable by clients and may either be a hostname or IP address. When blank
; the first public IP address that the simulator service is bound to is used. 
SimulatorAddress=localhost

; The address to which the server will bind the simulator and router services. If left
; blank, all detected network interfaces will be used.
;BindAddress=1.2.3.4

..
..

RouterPort=4242            ; Port to listen to router connections (login screen connect)
SimulatorPort=4300         ; Port to listen for simulator connections
HTTPListenPort=8080        ; Port to listen to HTTP requests.  Set as zero to disable.


```

The default configuration will work fine if you are setting up a server for your own use ('localhost'), 
but if you wish to make the server available to others over the internet, you will at least need to 
set *SimulatorAddress*. If you are running multiple servers on the same machine, you can either :-

 * Configure an IP address for each server, and use *BindAddress* in each instances ServerConfig.txt to
   bind to that address [RECOMMENDED].
 * Use the same IP address for each instance, but change *RouterPort*, *SimulatorPort* and *HTTPListPort*  in each instances ServerConfig.txt so that ports do not conflict.
 
#### Preparing GClient Assets

Having a running EE server on it's own in not much use, you will also need the *Client Assets* (i.e. all 3D models, 
Music, Sounds, Client Logic and more).

##### Client Assets

For various reasons we do not currently make this publically available. If you need assets, please contact us and we can provide you with a pre-built bundle.

###### Pre-built Client Assets

You will be provided with a file name *iceee-assets.zip*.

Assuming you are starting from the server source directory, it is recommended you unzip this file into the same parent folder as the server source. This way the default file locations in *ServerConfig.txt* are all ready setup as needed and ready to go.

```
cd ..
unzip /path/to/iceee-assets.zip
cd iceee
```

This will go to the parent folder, unzip the assets for the appropriate version of the game, and then change directory back into the server source folder (if you cloned the server source to somewhere other than 'iceee' folder, change accordingly).


### Running The Server

You now have everything you need to run the server. Assuming you are in the server source directory, run :-

```
SOURCE/Server/vald
```

The server will now start up with a default logging level of INFO output to the console.

### Creating The User Accounts

Before logging on to the server using the client, you will need to create a user. The default configuration uses a built in user database.

TODO
 

