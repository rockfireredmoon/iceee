This is the Git repository for the server data for the IceEE Planetforever Server. Changes on the dev server will periodically be pushed to this, as will changes by developers and their local server. This a subset of the data required for a full working server, it consists of most of the static data (except notably assets). 

 * AIScripts
 * Data
 * Instance
 * ItemMod
 * Loot
 * Packages
 * Scenery
 * SOURCE
 * SpawnPackages
 * VirtualItems
 * ZoneDef
 
## Source Modifications
 
 I will try to keep an overview of changes to the server source it was released.
 
  * Allow scripts to use Tab (\t) characters so improve compatibility and readability of scripts.
  * Add SO_REUSEADDR socket option so the server can be started and stopped quickly without having to wait for sockets to timeout.
  * Added ability to choose address for server to bind to. This allows multiple servers to be run on the same host without making client modifications by multihoming the host (mulitple IP addresses). Each server has it's own ServerConfig.txt that specifies the address to bind to, instead of listening to all addresses on the host. 
  * Added new instance script command 'despawn' to remove spawns given their PropID
  * Added new ability action 'RemoveHealthBuff' that can remove BONUS_HEALTH buffers (and con and health)
  * Added 'Guilds', kind of like clans, but fixed groups in game, opening quests and other guild specific content.
  
