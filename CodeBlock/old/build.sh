#!/bin/bash

clear
echo "Austin Arnold Code Block for 3-axis Underwater Sensor"
echo "Demonstration Script"
echo "2/11/26"
echo "Folder Size:"
du -sh   ~/Project/SigGen 
echo -e "\n"
echo "Compiling DSP program"
g++ -g DSP.cpp -o DSP
echo "Running DSP server in background"
./DSP &
echo "Running Signal Generator Client..."
echo -e "\n"

echo -e "\n"
echo "Checking SPI configurations..."
ls /dev/*spi*
echo -e "\n"

Waves="Sine"
TickPeriod=0.0002
Mode=0
Noise=0.1
Duration=10
Generator="CustomSet"
XAMP=4
XFREQ="pi/2"
YAMP=2
YFREQ="pi/3"
ZAMP=5
ZFREQ="pi/24"

echo "Signal Generator running with parameters:"
echo "$Waves Wave"
echo "$TickPeriod (s) = Tick Period"
echo "$Mode = Mode (0 Finite, 1 Continous)"
echo "$Duration (s) = Duration"
echo "$Generator = Wave Generation Mode"
echo "$Noise = Noise Level"
echo "$XAMP = X Amplitude"
echo "$XFREQ = X Frequency"
echo "$YAMP = Y Amplitude"
echo "$YFREQ = Y Frequency"
echo "$ZAMP = Z Amplitude"
echo "$ZFREQ = Z Frequency"
echo -e "\n\n"

python signals.py <<EOF
$Waves
$TickPeriod
$Mode
$Duration
$Generator
$Noise
$XAMP
$XFREQ
$YAMP
$YFREQ
$ZAMP
$ZFREQ
EOF
