#!/usr/bin/env bash 

echo "initializing the env"

ZEPHYR_BASE="$HOME/zephyr/zephyrproject" 

echo $ZEPHYR_BASE
source $ZEPHYR_BASE/.venv/bin/activate

source $ZEPHYR_BASE/zephyr/zephyr-env.sh 

