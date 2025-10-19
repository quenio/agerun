#!/bin/bash
# Verify checkpoint gate conditions before proceeding
# Usage: gate-checkpoint.sh <command_name> <gate_name> <required_steps>
# Returns: 0 if gate passes, 1 if gate fails
# Replaces 3-4 line gate verification pattern across commands

set -e
set -o pipefail

# Pass all arguments directly to checkpoint-gate.sh
bash scripts/checkpoint-gate.sh "$@"
