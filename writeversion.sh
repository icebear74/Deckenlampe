#!/bin/bash
# Script to update version with git commit hash
# Based on Panelclock repository pattern

# Get current git commit hash (short form)
COMMIT_HASH=$(git rev-parse --short HEAD)

# Update Version.h with the current commit hash
sed -i -E 's/(#define[[:space:]]+DECKENLAMPE_VERSION[[:space:]]+"[0-9]+\.[0-9]+\.[0-9]+-)[A-Za-z0-9]+(".*)/\1'"${COMMIT_HASH}"'\2/' Deckenlampe/Version.h

echo "Updated Version.h with commit hash: ${COMMIT_HASH}"
