#! /bin/bash                                                                                                  

MACHINE=$1
PORT=$2

sleep 1 
for ((i=1; i<=$NUM_PLAYERS; i++))
do
./player $MACHINE $PORT & 
done

wait