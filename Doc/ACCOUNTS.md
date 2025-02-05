# Game Accounts

The server is for an MMORPG, with emphasis here being on *Muiltiplayer*. Each player will require an account, and you will also need some security separating your *Administrators*, *Sages* and *Players*.

## First Time Setup

If you are just setting up your first [server](SERVER.md), then you will need to create some accounts.

First off, create an administrator account :-
 

```
eeaccount create admin 'mysecret' admin --roles=administrator
```

You can then create accounts for your Sages (GMs) :-

```
eeaccount create sage1 'mysecret' sagegrove1 --roles=sage
```

And finally, you might want to create a few accounts for ordinary players.

```
eeaccount create aplayer 'mysecret' playergrove1
```

You can of course just use this method to create and manage all your player accounts, but it could get tedious. So instead, you might consider using [Registion  Keys](#registration-keys), or even full [Website Integration](#website-integration).

## Other commands

The `eeaccount` command also provides some other commands for listing accounts, changing passwords and roles etc. Run a command without its arguments to help a little more help.

```
eeaccount create - create new accounts
eeaccount password - change passwords
eeaccount show - show account details
eeaccount list - list
eeaccount roles - show or change account roles
eeaccount delete - delete accounts
eeaccount groves - show account grove details
eeaccount remove-grove - remove player groves
eeaccount keys - list currently available registration keys
eeacccont genkeys - generate a new set registration keys
```

## Registration and More On Authentication

## Registration Keys

This method of registration has been around since Grethnefar's Planet Forever. One advantage it has is that it can be used without the complication of relying on email. 

 * Sets of registration keys are generated manually by an administrator when needed.
 * A player requests access to the server from a game administrator
 * If accepted, the game administrator gives one of generated keys to the player
 * The player uses their browser to go to [http://your-server-domain.com/CreateAccount.html](http://your-server-domain.com/CreateAccount.html) (or HTTPS if setup).
 * The player uses  their registration key, along with their own chosen username and password to create account.
 * The player can then login with their chosen username and password to the game.
 * The player can reset their password using the same registration by using their browser to go to [http://your-server-domain.com/ResetPassword.html](http://your-server-domain.com/ResetPassword.html) (or HTTPS if setup).
 
This works well enough, but has a few disadvantages too.

 * The player  must "remember" their registration key if they ever want to reset their own  password themselves. This is probably less useful than it might seem. If a player cannot hold of their password (which might actually be memorable in some way), then how are  they going to keep hold of their registration key?
 * It requires that game administrators manage the dolling out of registrating keys to avoid the same key being given out to more than one player at a time. *Keys are not removed from the available list until the player actually uses them.*
 * New sets of keys must be generated when needed.
 
In order to list the keys currently available, you can use the `eeacount` command.

You can also direct players to the mini player portal, available at the root of the server  (index.html), at [http://your-server-domain.com/](http://your-server-domain.com/)

```
eeacount keys
```
 
By default, NO keys are generated, so if the above command returns nothing, you must generate your first set. 

```
eccount genkeys 50
```

Will generate and display set of 50 keys. A subsequent run of `eeaccount keys` will then show these new keys too.

## Website Integration

*TAWD* is designed to be integrated with external authentication mechanisms, such as that used by our official website and game server. It can also retrieve certain static and variable game data live.

Configuration and usage of these features is being documented on the [API](API.md) page.