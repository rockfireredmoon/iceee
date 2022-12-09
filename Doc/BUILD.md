# Building

You are here if you wish to build and configure a server from source. If you just wish to play Earth Eternal,
run a standard private server, you should instead read [this](SERVER.md).

## Introduction

In order to build and run a server, the basic steps are :-

1. Install the pre-requisites.
2. Compile the server source.
3. Configure the server and create required files.
4. Obtain game data and client assets.
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
mkdir -p build/default
meson build/default --default-library=static 
```

#### Compile

Now compile the source :-

```bash
cd build/default
ninja compile
```

### Configuring The Server

Before starting the server, you will need to configure it for your environment. Follow the instructions on the [Configuration](CONFIGURATION.md) page, copying the sample configuration files and adjusting for your environment.

### Preparing Game Server Data and Client Assets

Having a running EE server on it's own in not much use, you will also need the *Game Server Data* (i.e. all static data such as Quests, NPC appearances, Scenery and more), and the *Client Assets* (i.e. all 3D models, 
Music, Sounds, Client Logic and more).

#### Game Server Data

We currently have two different Game Server Data sets that will run on this server. They are both stored in a separate GitHub repository - https://github.com/rockfireredmoon/iceee-data. One is for 'The Anubian War' (currently under development), and one for 'Valkal's Shadow' (our current live and active server).

Assuming you are starting from the server source directory, it is recommended you clone the game server data repository into the same parent folder as the server source. This way the default file locations in *ServerConfig.txt* are all ready setup as needed and ready to go.

#### For Valkals Shadow on TAW Engine

If you want to work on Valkal's Shadow (on TAW engine), then clone the *valkals_shadow* branch. 

```
cd ..
git clone -b valkals_shadow https://git@github.com/rockfireredmoon/iceee-data.git
cd iceee   
```

#### For The Anubian War on TAW Engine

```
cd ..
git clone https://git@github.com/rockfireredmoon/iceee-data.git
cd iceee   
```

Both of these will go to the parent folder, clone the server data for the 'TAW' version of the game, and then change directory back into the server source folder (if you cloned the server source to somewhere other than 'iceee' folder, change accordingly).

#### Client Assets

As with the game server data, we have two different versions of the Client Assets. Again, they are both stored in a Git repository under different branches. However, for various reasons we do not currently make this publically available. If you need assets, please contact us and we can provide you with a pre-built bundle.

##### Pre-built Client Assets

You will be provided with a file name *iceee-assets.zip*.

Assuming you are starting from the server source directory, it is recommended you unzip this file into the same parent folder as the server source. This way the default file locations in *ServerConfig.txt* are all ready setup as needed and ready to go.

```
cd ..
unzip /path/to/iceee-assets.zip
cd iceee
```

This will go to the parent folder, unzip the assets for the appropriate version of the game, and then change directory back into the server source folder (if you cloned the server source to somewhere other than 'iceee' folder, change accordingly).


##### Compiled Client Assets

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
build/default/tawd -C
```

The server will now start up with a default logging level of INFO output to the console.

### Account Setup

Before logging on to the server using the client, you will need to create a user. The default configuration uses a built in user database.

See [Accounts](ACCOUNTS.md) for how to setup the basics. Take note the `eeaccount` command is not yet installed, and can be found in `build/default`.


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

Again, working from build workspace 'build/default' :-

```bash
cd ..
meson configure --default-library=static -Dprefix=/usr -Dlocalstatedir=/var -Dsysconfdir=/etc -Dlocalconfigdir=/etc/tawd
```

NOTE: Old versions of Meson use a slightly different command that must be run inside `build/default`.

```bash
mesonconf --default-library=static -Dprefix=/usr -Dlocalstatedir=/var -Dsysconfdir=/etc -Dlocalconfigdir=/etc/tawd
```

If you want to install as a service (e.g Windows Service, SystemD Service etc), then add this property as well:-

```bash
-Dservice=true
```

Finally run the install. You will probably need to run this as administrator if the files are installed in system locations (the default) :-

```bash
cd build/default
sudo meson install
```
