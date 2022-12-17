# Game Configuration

This page describes how to adjust various tunable aspects of the game, such as level caps, fall damage, looting parameters and lots more.

It is not required to restart the server when these are changed, they are applied immediately. 

There are a couple of different ways to access the settings.

 * From inside the game client, logged on as an *Adminstrator* or *Developer* using the `gameconfig`  [In Game Command](IN_GAME_COMMANDS.md). This is recommended.
 * Directly in the database via any Redis client.
 
When configuration is set, it applies to all nodes in the cluster.

## In Game Command

This command can be used in 4 different ways. Firstly, to view the current configuration simply use :-

```
/gameconfig
```

The value of each setting will be display in the chatbox, one per line in the format `[Name]=[Value]`. 

The second form will the value of a single setting. E.g.

```
/gameconfig FallDamage
```

To set a value, supply it as the 2nd argument.

```
/gameconfig FallDamage true
```

The final form of this command is used to reload the configuration if it has been externally changed (i.e. directly in the database). 

```
/gameconfig reload
```

All usages of this command will be propagated to other nodes in the cluster automatically and immediately.

## Directly In Database

To do this you will require `redis-cli` or some kind of [GUI Redis client](https://redis.com/blog/so-youre-looking-for-the-redis-gui/).

Here I will assume you are using `redis-cli`, as it will already be available as you have installed Redis itself.

The game configuration is stored in a single *Key* (`GameConfig`) as a *Hash*. This is simply a table of names and values. 

*Note, all values here are stored as strings. We do not make use of other primitive types available such as booleans, integers etc*

First, start the Redis CLI. Here I am doing this from the same host where the Redis instance is installed, but you can use the tool remotely too (see `redis-cli --help`).

```
root@eeserver:~# redis-cli
127.0.0.1:6379> 
```

To see all of the current configuration. Use `HGETALL`. Each configuration item appears over two lines. The first line being the key, the second being the value.

```
127.0.0.1:6379> HGETALL GameConfig
1) "AllowEliteMob"
2) "true"
3) "MegaLootParty"
4) "true"
..
..
..
```

*Note, only configuration values that have changed will appear here. If an expected key is not seen, then it has been set and is currently at it's default value. See the table of configuration keys below for what the default value is.*

To set a value use the `HSET` command.

```
127.0.0.1:6379> HSET GameConfig MegaLootParty "true"
```

To get a value, use the `HGET` command.

```
127.0.0.1:6379> HGET GameConfig MegaLootParty
"true"
```

For any changes you might make directly as above, you must inform the cluster of the change in configuration before they are applied. Use the `/gameconfig reload` *In Game Command* to do so.

## The Configuration

This section describes all of the available keys used for game configuration and their default values.

| Key | Default Value | Description |
| --- | --- | --- |
| **AllowEliteMob** | true | If true, mobs may spawn as elite variants. |
| **MegaLootParty** | false | For fun and testing, when true, everything always drops, usually more than once. |
| **FallDamage** | false | If true, fall damage is enabled |
| **BuybackLimit** | 32 | The maximum number of items that will be held in the "Buyback" tab at vendors. |
| **Clans** | true | Whether clans are enabled or not |
| **ClanCost** | 10000 | How much is costs (in copper) to form a clan |
| **OverrideStartLoc** | *Blank* | When *Blank*, new characters will be placed at a zone and location decided by the static data (e.g. "Valkals Shadow" or "The Anubian War" data sets). When not blank, manually defines the starting zone, xyz and rotation of all new characters. The string is in the format `[zoneId];[x],[y],[z],[rotation]`. |
| **CapExperienceLevel** | 70 | At what level should the server start to limit the amount of experience that would be added for a given event (e.g. kill). When this occurs, the amount to add will be limited to whatever is set in **CapExperienceAmount** set below. When set to zero, this cap will never occur. |
| **CapExperienceAmount** | 0 | When **CapExperienceLevel** is reached, what is the maximum amount of XP that can be added for any event (e.g. kill). If this is set to **0**, then no more experience can be added. |
| **CapValourLevel** | 0 | Currently unused until valour system is activated. Will be a hard cap on the valour level. |
| **AprilFools** | false | Whether or not to activate aprils fools jokes. This useless bit of a fun is being left in the server in homage to the player it was created for, "Disaster Master". It changes the players appearance to something hard coded while the joke is active. |
| **AprilFoolsName** | *Blank* | When **AprilsFools** is active, also changes the player name to that specified here. Another bit of fun (victimisation) of that same player. |
| **AprilFoolsAccount** | 0 | The account number the above two settings apply to. |
| **VaultDefaultSize** | 16 | Number of vault slots that all characters have.  If characters have not purchased any slots at all, this amount will still be available. This is clamped to a hard coded maximum of **120** |
| **VaultInitialPurchaseSize** | 8 | Newly created characters will be given this many free slots (considered as purchased space). |
| **GlobalMovementBonus** | 0 | If nonzero, all objects placed into a instance (players, mobs, NPCs, etc) will gain this default modifier to run speed. |
| **DexBlockDivisor** | 0 | Points of dexterity may provide a bonus chance to block physical attacks. |
| **DexParryDivisor** | 0 | Points of dexterity may provide a bonus chance to parry physical attacks. |
| **DexDodgeDivisor** | 0 | Points of dexterity may provide a bonus chance to dodge physical attacks. |
| **SpiResistDivisor** | 0 | Points of spirit may provide bonus resistance to certain elemental attacks. |
| **PsyResistDivisor** | 0 | Points of psyche may provide bonus resistance to certain elemental attacks. |
| **CustomAbilityMechanics** | false | If true, certain abilities may be processed with custom mechanics differently than a classic official server might. |
| **NamedMobDropMultiplier** | 4.0 | All mobs marked as *Named* will receive a drop rate bonus for randomized items. |
| **NamedMobCreditDrops** | 1 | All mobs marked as *Named* will drop credits (when the player is at or below level, bonus given for parties). |
| **LootMaxRandomizedLevel** | 50 | The randomizer cannot generate loot above this level for typical mobs. |
| **LootMaxRandomizedSpecialLevel** | 55 | The randomizer cannot generate loot above this level for "special" mobs |
| **LootNamedMobSpecial** | true | If true, named mobs are considered for the special item level cap. |
| **LootMinimumMobRaritySpecial** | 2 | The minimum quality level for a mob to be considered special. |
| **HeroismQuestLevelTolerance** | 3 | How many levels above the quest level that the player is allowed to be to receive full heroism. |
| **HeroismQuestLevelPenalty** | 4 | Points of heroism to lose per level if over the quest tolerance level. |
| **ProgressiveDropRateBonusMult** | 0.0025,<br />0.005,<br />0.01,<br />0.02 | Comma separated value, which each element being for on of the *Rarity* values (0,1,2 or 3). Additive amount to increase instance drop rates per kill by a creature of a certain rarity. |
| **ProgressiveDropRateBonusMultMax** | 2.0 | The maximum instance drop rate bonus from additive kills. |
| **DropRateBonusMultMax** | 200.0 | The maximum drop rate bonus multiplier that any kill may have.  This affects the absolute total after all drop rate calculations have been applied. |
| **NameChangeCost** | 300 | Number of credits a last name change costs. |
| **MinPVPPlayerLootItems** | 0 | Minimum number of items that will be dropped by the player after a PVP fight. |
| **MaxPVPPlayerLootItems** | 3 | Maximum number of items that will be dropped by the player after a PVP fight |
| **MaxAuctionHours** | 168 | Maximum number of hours an auction can last. |
| **MinAuctionHours** | 1 | Minimum number of hours an auction can last. |
| **PercentageCommisionPerHour**  | 0.208333333 | Percentage to take per hour. |
| **MaxAuctionExpiredHours** | 24 | Maximum number of hours an auction can be expired. |
| **MaxNewCreditShopItemDays** | 31 | Maximum number of days an item in the credit shop is considered 'New'. |
| **EnvironmentCycle** | Sunrise=05:30,<br />Day=08:30,<br />Sunset=18:00,<br />Night=20:30 | String that determines schedule for day -> night changes. |
| **UseAccountCredits** | true | If true, credits will be stored at the account level rather than per character and shared across all characters. |
| **UseReagents** | true | If true, reagents are enabled and required for certain abilities and scrolls. |
| **UsePersistentBuffs** | true | If true, active buffs will be saved and restored on next login. |
| **UsePartyLoot** | true | Whether to allow party loot. |
