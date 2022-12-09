# Running Your Own Server

You are here if you wish to run your own private server for any Earth Eternal game such as 
[Valkal's Shadow](https://www.theanubianwar.com/valkals-shadow).  

## Recommended Setup

The quickest way to get a running server is to use a supported Linux distribution along with a one of the TAW projects official package repositories.

Ideally, and especially if this is your first server, you should install onto a completely fresh operating system. Consider using a Virtual Machine and a tool such as [VirtualBox](https://www.virtualbox.org/) to practice.

We currently support the following operating systems and versions, with more on the way.

 * Debian 11
 * Fedora 37
 * openSUSE Tumbleweed
 * Raspbian 11
 * Ubuntu 22.04
 
*If you would like us to support another operating system or version, let us know. We should be able to add new  distributions relatively easily.*
 
You will need 3 packages for a working server. Click the links to download the appropriate packages and follow the instructions. We recommended setting up a repository so you receive updates.

 1. [tawd](https://software.opensuse.org//download.html?project=home%3Aemerald.icemoon&package=tawd). This is the actual server and server tools. 
 1. [tawd-data-valkals-shadow](https://software.opensuse.org//download.html?project=home%3Aemerald.icemoon&package=tawd-data-valkals-shadow). This contains the static game data such as quests, NPC definitions and lots more. Packages for other editions of the game will be made available at a later date.
 1. [tawd-client-assets-valkals-shadow](https://software.opensuse.org//download.html?project=home%3Aemerald.icemoon&package=tawd-client-assets-valkals-shadow). This contains the 3D models that the game client will download as and when it needs them. Again, packages for other editions of the game will be made available at a later date.
 
### What's Next

Now the packages are installed, you simply have to start the *Service*. 

```
systemctl start tawd
```

You should now move on to creating some [game accounts](ACCOUNTS.md), configuring your server for first time use. 

If you encounter any problems, review the [Configuration](CONFIGURATION.md) files and adjust accordingly. Our packages install a configuration that should work "out of the box" for the simplest setup, but anything more advanced will require manualy configuration.
 
### Other Operating Systems

If we do not yet have packages available for your system, you can [Build From Source](BUILD.md). The source should compile on any recent version of Linux or Windows.

