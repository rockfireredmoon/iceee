# About TAWD - An Earth Eternal Server

## Introduction

Earth Eternal is an abandoned MMORPG published and run by Sparkplay Media from 2007 to 2010. Since then it has been in various hands, but never achieved mainstream success. 

During this time, an enterprising player managed to develop an alternative server which was eventually brought online as a private server enjoyed by a small community for a few years before he stood down. At this point he released the source to the community to do with as they will. For a full history, see our website http://www.theanubianwar.com/  

This is the Git repository for the server data and source for TAWD, the continuation of the IceEE project (SOAD, VALD), which itself is a continuation Grethnefar's Planetforever Server. Changes on the dev server will periodically be pushed to this, as will changes by developers and their local server. 
 
### Source Modifications
 
 In addition to the large number of changes in the earlier IceEE servers, TAWD is an attempt to address
 some architectural problems, add new mechanics and web site integration and more.
 
  * All user data now stored in a Redis instance (https://redis.io). This is key-value database with
    built-in high availability and clustering. 
  * Shards are implemented. Redis provides a pub/sub messaging API that is used to achieve this.
  * HTTP server component replaced with CivetWeb, an embeddable C/C++ HTTP server. This brings HTTPS support
    and other performance, scalibity, security and maintenance advantages. 
  * Replaced logging system with easylogging
  * New quest mechanics (ad-hoc quests, 'outcomes')
  * Abstracted message / query system
  * Clan support
  * HTTP API for website integration
  * Other tidying
  * New Meson based build

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

*Gloom Horizon* - Ambient music used in Grunes Tal undead region

*Crossing The Chasm* - Ambient music used in Swineland region

*Dark Times* - Activate music used in Bloodkeep

*Land Of Phantoms* - Ambient music used in Bloodkeep

*Some Amount Of Evil* - Ambient music used in Bloodkeep

*Killers* - Ambient music used in penultimate fight scene in Bloodkeep

*Chee Zee Cave* - Music used in Southend Passage

*Night Cave* - Music used in Forest Cave

*Curse Of The Scarab* - Music used in Djinn Temple

*Dragon And Toast* - Music used in Fangarian's Lair

*Grim Idol* - Fight Music used in Fangarian's Lair

*The Hive* - Background Music used in Skrill Queen Lair

*Constance* - Fight Music used in Skrill Queen Lair

*Christmas Rap* - Ambient music used for Winter Dawning region

*Wish Backgroud*  - Ambient music used for Winter Dawning region

Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 3.0
http://creativecommons.org/licenses/by/3.0/

*Tap* - Finale in Valkal's Bloodkeep
Music: Alexander Nakarada (www.serpentsoundstudios.com)
Licensed under Creative Commons: By Attribution 4.0 License
http://creativecommons.org/licenses/by/4.0/

Lots of sound effects used as background or spot effects throughout the
game came from freesound.org. All of these are 
Licensed under Creative Commons: By Attribution 3.0

*Forest Evening* - cediez - https://www.freesound.org/people/cediez/ 

*Kankbeeld Horror Pack* - Kankbeeld - http://www.freesound.org/people/klankbeeld/

*Dungeon Ambiance* - https://freesound.org/people/phlair/sounds/388340/

*Lava loop* - Audionautics - https://freesound.org/people/Audionautics/sounds/133901/

*Dripping Cave* - dobroide - https://freesound.org/people/dobroide/sounds/396314/

*Thunder Pack* - hantorio - https://freesound.org/people/hantorio/packs/7640/ 

*10 Second Countdown* - thomasevd - https://freesound.org/people/thomasevd/sounds/202193/

 And some from https://tabletopaudio.com
 Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.

*In The Shadows* - Ambient background 

## Configuring A Server From This Repository

This a subset of the data required for a full working server, it consists of the server source, and most of the static data (except notably assets).

In order to run a server, the basic steps are :-

1. Install the pre-requisites.
2. Compile the server source
3. Configure the server and create required files.
4. Start the server daemon.

### Compile Pre-requisites

Most 3rd party dependencies are included in the source tree, and you do not need to do anything.
But, there are a couple that must install first.

 * libcurl (https://curl.haxx.se/libcurl/). Used for outgoing network connectivity to the website and other services such as GitHub
   and mail servers.
 * OpenSSL (https://www.openssl.org/). Used only when SSL is enabled, this adds HTTPS functionality to the HTTP server.

### Run-time Pre-requisites

These are dependencies needed at run-time.

 * Redis (https://redis.io/). A running Redis instance is needed to store the user data and for communication between shards in a clustered setup.
  
### Compiling The Server

This project is distributed as an Meson (http://mesonbuild.com/) project. 

#### Preparing the source tree

Before compilation of the source tree can take place, the raw source tree from GitHub must
be processed, creating additional files to aid configuration of the source appropriate for
your platform.

If you have downloaded a source bundle, this step is not required so you can jump straight
to Configure The Source.  

You will need the following installed :-

* Meson (http://mesonbuild.com/)
* Ninja. Make system. https://ninja-build.org/
* C++11 Compiler and Linker (e.g. GCC 5+, MSVC 2015)

Other packages may be needed for various common libraries used. All of these should be
easily obtainable for your OS/Distribution. 

Run :-

```bash
meson builddir --default-library=static 
```

#### Compile

Now compile the source :-

```bash
ninja compile
```

### Configuring The Server

Before starting the server, you will probably want to configure it for your environment.

#### Configuring The Cluster (all configurations)

Regardless of whether or not you have multiple nodes, you will need to tell the server where your
Redis instance is located and how to connect to it.

This source module ships with sample configuration files that must be copied to their 'real' locations.
Do this for the local cluster configuration file :-

```
cp Local/Cluster.txt.sample Local/Cluster.txt
```

And then open up *Local/Cluster.txt* in your favourite text editor :-
 

```
; The unique name of this shard
ShardName=EE1

; A more descriptive name displayed in the shard selection screen (/shards)
FullName=My Local Earth Eternal server

; The address of this shards Redis instance
Host=127.0.0.1

; The port on which this shards Redis instance is listening
Port=6379

; The password to use to authenticate with the Redis instance. Leave blank or commented out if there is no password
;Password=asecret

```

This file is also used to set to the unique shard name and description. Do not worry about this if
you are only running a single shard.

#### Configuring The Cluster (multiple shards)

If you wish to run multiple shards in a cluster, there are a few different configurations. 

 * Multiple server instances pointing to a single shared Redis instance.
 * Multiple server instances, each with it's own Redis instance and replication setup between the Redis instances.
 * A combination of both.
 
*All replication configuration when multiple Redis instances are used  is done at the Redis layer. See https://redis.io/topics/replication*


Whenever you have multiple EE server instances, each must have their own *Local* configuration directory.
Anything in this directory is specific to this node only.

#### Configuring Server Behaviour

All other server behaviour is configured using the Local/ServerConfig.txt file. As with all other configuration files, this is shipped as a ServerConfig.txt.sample, and must be copied and edited before
use.

```
cp Local/ServerConfig.txt.sample Local/ServerConfig.txt
```

Then edit *Local/ServerConfig.txt with your favourite text editor. This file contains many options,
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
 
NOTE: If you are setting up a cluster, MOST other options should the SAME on all nodes. For example, you do not want each shard to have different values for *CapExperienceLevel*, but something like *MOTD_Message* is OK. This will eventually be fixed so such configuration is moved into the cluster database for all shards to share.

#### Logging Configuration

Logging is provided by EasyLogging (https://muflihun.github.io/easyloggingpp/). See this site for detailed information on how logging is configured.

Server log configuration files are in the *Local* configuration directory. As with other configuration files,
before use you will need to copy the sample file :-

```
cp Local/LogConfig.txt.sample Local/LogConfig.txt
```

If you are just starting out, there should be no need to change anything in this file. All logs by default will go to the *Logs* folder. 

#### Preparing Game Server Data and Client Assets

Having a running EE server on it's own in not much use, you will also need the *Game Server Data* (i.e. all static data such as Quests, NPC appearances, Scenery and more), and the *Client Assets* (i.e. all 3D models, 
Music, Sounds, Client Logic and more).

##### Game Server Data

We currently have two different Game Server Data sets that will run on this server. They are both stored in a separate GitHub repository - https://github.com/rockfireredmoon/iceee-data. One is for 'The Anubian War' (currently under development), and one for 'Valkal's Shadow' (our current live and active server).

Assuming you are starting from the server source directory, it is recommended you clone the game server data repository into the same parent folder as the server source. This way the default file locations in *ServerConfig.txt* are all ready setup as needed and ready to go.

```
cd ..
git clone https://git@github.com/rockfireredmoon/iceee-data.git
cd iceee   
```

This will go to the parent folder, clone the server data for the 'TAW' version of the game, and then change directory back into the server source folder (if you cloned the server source to somewhere other than 'iceee' folder, change accordingly).

##### Client Assets

As with the game server data, we have two different versions of the Client Assets. Again, they are both stored in a Git repository under different branches. However, for various reasons we do not currently make this publically available. If you need assets, please contact us and we can provide you with a pre-built bundle.

###### Pre-built Client Assets

You will be provided with a file name *iceee-assets.zip*.

Assuming you are starting from the server source directory, it is recommended you unzip this file into the same parent folder as the server source. This way the default file locations in *ServerConfig.txt* are all ready setup as needed and ready to go.

```
cd ..
unzip /path/to/iceee-assets.zip
cd iceee
```

This will go to the parent folder, unzip the assets for the appropriate version of the game, and then change directory back into the server source folder (if you cloned the server source to somewhere other than 'iceee' folder, change accordingly).


###### Compiled Client Assets

If you have access to Client Assets Git repository, you can compile the assets from source. There are a number of tools you will need installed, see the README.md that will be cloned along with the source.  

```
cd ..
git clone [REDACTED]
cd iceee-assets
ant
cd ../iceee   
```

### Running The Server

You now have everything you need to run the server. Assuming you are in the server source directory, run :-

```
SOURCE/Daemon/tawd
```

The server will now start up with a default logging level of INFO output to the console.

### Creating The User Accounts

Before logging on to the server using the client, you will need to create a user. The default configuration uses a built in user database.

First off, create an administrator account :-
 

```
SOURCE/Tools/eeaccount -i create admin 'mysecret' admin --roles=administrator
```

You can then create accounts for your Sages (GMs) :-

```
SOURCE/Tools/eeaccount -i create sage1 'mysecret' sagegrove1 --roles=sage
```

And finally acccounts for ordinary players

```
SOURCE/Tools/eeaccount -i create aplayer 'mysecret' playergrove1
```

There are various other ways by which accounts by created, such as via HTTP calls. These are intended for integration with web sites. 


### Server Daemon Command Line Options

There are a number of options available to aid debuggging and running the process from service scripts and the like. 

| Option | Arguments | Description |
| ------ | --------- | ----------- |
| -d | None | Daemonize the server. Once initialized, the process will be forked and placed into the background |
| -C | None | Output all logging to the console as well as log files. |
| -p | [path] | Location to PIDFILE. The process ID of the server is written to this file once known |
| -c | [path] | Location to a *Local* configuration directory (that contains ServerConfig.txt, LogConfig.txt and Cluster.txt) |
| -I | None | Flush log output immediately. Ordinarilly this is buffered to aid performance, but disabling this can help in some debugging situations |
| -L | [loglevel] | Set the default log level. Position values for <loglevel> include info, debug, error, fatal, trace, verbose or warning |

## Installation

At some point, you will probably want to actually the server, or create installable packages for your
distribution.

Again, working from build workspace 'builddir' :-

```bash
cd ..
meson configure --default-library=static -Dprefix=/usr -Dlocalstatedir=/var -Dsysconfdir=/etc -Dlocalconfigdir=/etc/tawd
```

NOTE: Old versions of Meson use a slightly different command that must be run inside the builddir.

```bash
mesonconf --default-library=static -Dprefix=/usr -Dlocalstatedir=/var -Dsysconfdir=/etc -Dlocalconfigdir=/etc/tawd
```

If you want to install as a service (e.g Windows Service, SystemD Service etc), then add this property as well:-

```bash
-Dservice=true
```

Finally run the install. You will probably need to run this as administrator if the files are installed in system locations (the default) :-

```bash
cd builddir
sudo meson install
```
