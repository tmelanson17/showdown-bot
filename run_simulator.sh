#!/bin/bash

SHOWDOWN_BINARY=/Users/tjmelanson/development/downloaded-code/pokemon-showdown/pokemon-showdown

# TODO: Feed the config into the bot 
P1_TEAM=$(cat config/sample_team_packed.txt)
P2_TEAM=$(cat config/sample_team_packed.txt)
FORMAT=$(cat config/format_config.json) 

COMMAND=">start $(echo $FORMAT)
>player p1 {\"name\": \"P1\", \"team\": \"${P1_TEAM}\"}
>player p2 {\"name\": \"P1\", \"team\": \"${P2_TEAM}\"}
"
echo "$COMMAND"
echo "$COMMAND" | ${SHOWDOWN_BINARY} simulate-battle
