# Running Your Own Server

You are here if you wish to run your own private server for any Earth Eternal game such as 
[Valkal's Shadow](https://www.theanubianwar.com/valkals-shadow).

There are 3 ways to get a server up and running.

 1. [All In One Virtual Machine Builds](#all-in-one-virtual-machine-builds). *Recommended*. Use a VM (Virtual Machine) or generic ISO with your hypervisor or cloud provider of choice.
 1. [Native Packages](#native-packages). We build packages for several Linux distributions, for example Debian. The virtual machine and  ISO builds use these packages.
 1. [Build From Source](#build-from-source). If nothing else fits, build from source.
 
## All In One Virtual Machine Builds

These builds contains everything needed to run the game, already setup for you. The only thing you need to do is follow the instructions  in the [What Next](#what-next) section below.

We provide OVA, KVM, HyperV, Docker and more to come. You can also use the generic ISO image either on real hardware or most hypervisors. 

See the [tawd-vm](https://github.com/rockfireredmoon/tawd-vm) project for more details including the latest download locations. 

## Native Packages

Another way to get a running server is to use a supported Linux distribution along with a one of the TAW projects official package repositories.

We currently support the following operating systems and versions, with more on the way.

 * Debian 11 or 12
 * Fedora 37
 * openSUSE Tumbleweed
 * Raspbian 11 or 12 (ARM, i.e. Rapberry Pi)
 * Ubuntu 22.04 or 24.04
 
*If you would like us to support another operating system or version, let us know. We should be able to add new  distributions relatively easily.*
 
You will need 3 packages for a working server. Click the links to download the appropriate packages and follow the instructions. We recommended setting up a repository so you receive updates.

 1. [tawd](https://software.opensuse.org//download.html?project=home%3Aemerald.icemoon&package=tawd). This is the actual server and server tools. 
 1. [tawd-data-valkals-shadow](https://software.opensuse.org//download.html?project=home%3Aemerald.icemoon&package=tawd-data-valkals-shadow). This contains the static game data such as quests, NPC definitions and lots more. Packages for other editions of the game will be made available at a later date.
 1. [tawd-client-assets-valkals-shadow](https://software.opensuse.org//download.html?project=home%3Aemerald.icemoon&package=tawd-client-assets-valkals-shadow). This contains the 3D models that the game client will download as and when it needs them. Again, packages for other editions of the game will be made available at a later date.
 
### What Next

Now the packages are installed, you simply have to start the *Service*. 

```
systemctl start tawd
```

You should now move on to creating some [game accounts](ACCOUNTS.md), configuring your server for first time use. 

If you encounter any problems, review the [Server And Cluster Configuration](SERVER_AND_CLUSTER_CONFIGURATION.md) files and adjust accordingly. Our packages install a configuration that should work "out of the box" for the simplest setup, but anything more advanced will require manualy configuration.

You may also wish to adjust some of the [Game Configuration](GAME_CONFIGURATION.md). These are settings that generally affect game play such as looting parameters, fall damage and lots more.
 
## Build From Source

If we do not yet have packages available for your system, you can [Build From Source](BUILD.md). The source should compile on any recent version of Linux or Windows.

