# Configuring The Server

This page gives a quick overview of the main configuration files. If you have installed our official packages, there should be no need to copy the sample configuration files, as defaults will have installed.

## Locating The Configuration Files

The configuration files could be in a few places, depending on how the server was installed.

 * If you have installed our official Linux packages, then you will find the active server configuration in `/etc/tawd`. 
 * If you have installed the server from source, they will be wherever the Meson option `localconfigdir` points to.
 * If you have built the server from source and have not yet installed it, the files will be in the root of the source directory, e.g. `Local` and you will need to copy the sample files. See below
 
## Copying The Sample Files

If you have built the server from source but not installed it, you will need to copy the sample files to their real location. Everyone else should skip this step and move on to *Cluster Configuration (single or multiple nodes)*

Configuration files for this setup are in the *Local* directory, copy each one.

```
cp Local/LogConfig.txt.sample Local/LogConfig.txt
cp Local/ServerConfig.txt.sample Local/ServerConfig.txt
cp Local/Cluster.txt.sample Local/Cluster.txt
```

## Editing The Files

Nowyou have located the files, you can should use your favourite text editor to open and review them.

### Cluster Configuration

All TAWD servers support and use clustering. If you have a single server, it is just a just of one. 
 
####Cluster Configuration (single or multiple nodes)

`Cluster.txt`

Regardless of whether or not you have multiple nodes, you will need to tell the server where your
Redis instance is located and how to connect to it.

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

### Cluster Configuration (multiple shards)

`Cluster.txt`

If you wish to run multiple shards in a cluster, there are a few different configurations. 

 * Multiple server instances pointing to a single shared Redis instance.
 * Multiple server instances, each with it's own Redis instance and replication setup between the Redis instances.
 * A combination of both.
 
*All replication configuration when multiple Redis instances are used  is done at the Redis layer. See https://redis.io/topics/replication*


Whenever you have multiple EE server instances, each must have their own *Local* configuration directory.
Anything in this directory is specific to this node only.

### Server Behaviour

`ServerConfig.txt`

All other server behaviour is configured using the Local/ServerConfig.txt file. As with all other configuration files, this is shipped as a ServerConfig.txt.sample, and must be copied and edited before
use.

This file contains many options, most of which are commented. For a new installation, the ones that most likely need to be changed are :-

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

### Logging Configuration

`LogConfig.txt`

Logging is provided by EasyLogging (https://muflihun.github.io/easyloggingpp/). See this site for detailed information on how logging is configured.

If you are just starting out, there should be no need to change anything in this file. All logs by default will go to the *Logs* folder. 