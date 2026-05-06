#!/bin/bash
TIME=120

PORT=20001
fuser -k $PORT/tcp

pkill DSP
pkill Frontend
pkill Transmit

FRONTENDIPC=1
DSPIPC=1
TRANSMITIPC=1
IPCCOMPLETE=1
TRANSMITSPI=1
FRONTENDSPI=0
FULLSPI=0
TIMING=0
CALCULATIONS=0
FINISHED=0

echo -e "Building new DSP code....\n"
echo "=======State of implementation==========="
echo "FrontendIPC is complete: $FrontendIPC"
echo "DSPIPC is complete: $DSPIPC "
echo "TransmitIPC is complete: $TRANSMITIPC"
echo "Full interprocess comunication complete: $IPCCOMPLETE"
echo "Transmit SPI is complete: $TRANSMITSPI"
echo "Frontend SPI is complete: $FRONTENDSPI"
echo "Full SPI is implemented: $FULLSPI"
echo "Timing is accurate and in sync: $TIMING"
echo "Calculations are accurate and DSP is complete: $CALCULATIONS"
echo "Code is complete: $FINISHED"
echo "========================================="
echo -e "\n\n\n"

g++ -g -Wall -Wextra DSP.cpp -o DSP
g++ -g -Wall -Wextra Frontend1.cpp -o Frontend1
g++ -g -Wall -Wextra Frontend2.cpp -o Frontend2
g++ -g -Wall -Wextra Transmit.cpp -o Transmit

echo "Starting in 3 seconds"
sleep 3

pids=()

./DSP &
pids+=($!)
./Frontend1 &
pids+=($!)
./Frontend2 &
pids+=($!)
./Transmit &
pids+=($!)

echo "Launched process with PIDS: ${pids[@]}"

sleep $TIME

echo "Killing all processes after $TIME seconds"
kill -9 "${pids[@]}" 2>/dev/null

wait "${pids[@]}" 2>/dev/null

echo "Cleaning up..."


echo "Done"
