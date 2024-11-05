#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
cargo build --release &>/dev/null

TARGET_DIR="$SCRIPT_DIR/target/release"
cd "$TARGET_DIR"
./deadlocked
