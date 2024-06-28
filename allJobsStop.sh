#!/bin/bash

# Get all running jobs
running_jobs=$(./jobCommander poll running | awk -F ',' '{print $1}')

# Get all queued jobs
queued_jobs=$(./jobCommander poll queued | awk -F ',' '{print $1}')

# Stop all running jobs
for job in $running_jobs; do
    if [ -n "$job" ]; then
        ./jobCommander stop "$job"
    fi
done

# Stop all queued jobs
for job in $queued_jobs; do
    if [ -n "$job" ]; then
        ./jobCommander stop "$job"
    fi
done
