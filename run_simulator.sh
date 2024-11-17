#!/bin/bash

SHOWDOWN_BINARY="/Users/tjmelanson/development/downloaded-code/pokemon-showdown/pokemon-showdown simulate-battle"

# create a temporary named pipe for input and output
PIPE=$(mktemp -u)
mkfifo $PIPE
# attach it to file descriptor 3
exec 3<>$PIPE
echo "stdin: $PIPE"

# Don't pipe stdout for now
#PIPEOUT=$(mktemp -u)
#mkfifo $PIPEOUT
#echo "stdout: $PIPEOUT"
# exec 4<>$PIPE
#rm $PIPEOUT


echo "Running showdown binary"
${SHOWDOWN_BINARY} <&3 # >&4 

# unlink the named pipe
rm $PIPE

exec 3<&-
