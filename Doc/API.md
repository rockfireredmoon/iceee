# API

The game server has an HTTP API that may be used to integrate with a community web site. Possibilities include, but are not limited to ..

 * OAuth2 authentication
 * Resetting passwords
 * Obtaining player and clan information and stats.
 * Obtaining the leaderboard.
 * Region and private chat
 * Auction House integration
 * Up-time and online player lists
 * Credit shop integration
 
The above are collectively known as the *TAWD API*.
 
It also supports the *Legacy API*. This is the API as used since Grethnefar's Planet Forever, and in Scourge of Abidan, and Valkal's Revenge. 

 * Creating accounts, resetting passwords and managing registration keys
 * Web Control Pane
 
## TAWD API
 
### General Protocol

All API calls must be authenticated. This is currently done using [https://en.wikipedia.org/wiki/Basic_access_authentication](HTTP BASIC authentication), and you should ensure that it is as safe as possible. 

 * Use a very strong password, 20 characters or more
 * Only use HTTPS  unless you are connecting from the same machine on localhost. 

TODO

## Legacy API

TODO

### Account Management

TODO