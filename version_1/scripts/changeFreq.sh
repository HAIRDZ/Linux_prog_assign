#!/bin/bash

baseDir="$(cd "$(dirname "$0")/.." && pwd)"

values=(1000000 100000 10000 1000 100)

for v in "${values[@]}"
do
    echo $v > "$baseDir/data/freq.txt"
    sleep 60
done
