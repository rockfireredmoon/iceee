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

And finally acccounts for ordinary players

```
eeaccount create aplayer 'mysecret' playergrove1
```

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

```

## External Authentication

*TAWD* is designed to be integrated with external authentication mechanisms, such as that used by our official website and game server. 

Configuration and usage of these features is beyond the scope of this document.