#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$REPO_ROOT"

if [ ! -d "dist/mverb" ]; then
    echo "Run ./scripts/build.sh first."
    exit 1
fi

ssh ableton@move.local "mkdir -p /data/UserData/schwung/modules/audio_fx/mverb"
scp -r dist/mverb/* ableton@move.local:/data/UserData/schwung/modules/audio_fx/mverb/
ssh ableton@move.local "chmod -R a+rw /data/UserData/schwung/modules/audio_fx/mverb"
