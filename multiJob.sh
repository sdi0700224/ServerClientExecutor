#!/bin/bash

# Check if at least one file is provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <file1> <file2> ... <fileN>"
    exit 1
fi

# Iterate over each provided file
for file in "$@"; do
    if [ -f "$file" ]; then
        # Read each line from the file and submit it as a job
        while IFS= read -r job; do
            if [ -n "$job" ]; then
                ./jobCommander issueJob "$job"
            fi
        done < "$file"
    else
        echo "File $file does not exist"
    fi
done
