#!/bin/bash

DIR="$(dirname "$0")"
FIFO="/tmp/ircserv_dump"
LOG="$DIR/result.log"

PID=$(pgrep -x ircserv | head -n 1)

if [ -z "$PID" ]; then
	echo -e "\033[31mircserv n'est pas lancé\033[0m"
	exit 1
fi

if [ ! -p "$FIFO" ]; then
	mkfifo "$FIFO"
fi

cat "$FIFO" > "$LOG" &
READER=$!

sleep 0.01

kill -SIGUSR1 "$PID"

wait "$READER"

cat "$LOG"
echo
echo -e "\033[32mResultat enregistré dans result.log\033[0m"
