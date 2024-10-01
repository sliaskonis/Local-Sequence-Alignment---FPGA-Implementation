#!/bin/bash

# Function to compile with DEBUG flag if DEBUG argument is passed
compile_with_debug() {
    if [ "$1" = "DEBUG" ]; then
        gcc -Wall -g lsal.c -O3 -mavx2 -o lsal -D DEBUG
        gcc -Wall -g lsal_opt.c -O3 -mavx2 -o lsal_opt -D DEBUG
        gcc -Wall -g lsal_opt_pad.c -O3 -mavx2 -o lsal_opt_pad -D DEBUG
    else
        gcc -Wall -g lsal.c -O3 -mavx2 -o lsal
        gcc -Wall -g lsal_opt.c -O3 -mavx2 -o lsal_opt
        gcc -Wall -g lsal_opt_pad.c -O3 -mavx2 -o lsal_opt_pad
    fi
}

# Check if DEBUG argument is passed
if [ "$1" = "DEBUG" ]; then
    compile_with_debug "DEBUG"
else
    compile_with_debug
fi

# Run script to get graphs
if [ "$1" = "DEBUG" ]; then
    echo "Running all programs..."
    echo "Running lsal..."
    ./lsal > lsal.txt
    echo "Running lsal_opt..."
    ./lsal_opt > lsal_opt.txt
    echo "Running lsal_opt_pad..."
    ./lsal_opt_pad > lsal_opt_pad.txt
else
    python3 get_graphs.py
fi

if [ "$1" = "DEBUG" ]; then
    # DO diff for all outputs and check if its okay
    diff expOut.txt lsal.txt
    diff expOut.txt lsal_opt.txt
    diff expOut.txt lsal_opt_pad.txt
fi

# Remove all output files
if [ "$1" = "DEBUG" ]; then
    rm lsal.txt
    rm lsal_opt.txt
    rm lsal_opt_pad.txt
fi

# Clean up compiled programs
rm lsal
rm lsal_opt
rm lsal_opt_pad
