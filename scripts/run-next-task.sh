#!/bin/bash

set -e

# Next Task Discovery Workflow Script
# This script runs the complete checkpoint-based task discovery workflow

# Initialize checkpoint tracking or show status if already initialized
./scripts/init-checkpoint.sh next-task '"Read Context" "Check Task Sources" "Discover Next Task"'

# Verify checkpoint tracking is ready
./scripts/require-checkpoint.sh next-task

# ============================================================================
# STAGE 1: Read Context (Step 1)
# ============================================================================

echo ""
echo "========== STAGE 1: Read Context =========="
echo ""

echo "Reading AGENTS.md and session context..."
echo ""

# Verify necessary files exist
if [ ! -f AGENTS.md ]; then
  echo "❌ ERROR: AGENTS.md not found"
  exit 1
fi

echo "✅ Context files loaded"

make checkpoint-update CMD=next-task STEP=1

# ============================================================================
# STAGE 2: Check Task Sources (Step 2)
# ============================================================================

echo ""
echo "========== STAGE 2: Check Task Sources =========="
echo ""

echo "Checking for tasks in:"
echo "  1. Session todo list (current execution context)"
echo "  2. TODO.md file (project tasks)"
echo ""

SESSION_TASKS=0
if [ -f .session_todos.txt ]; then
  SESSION_TASKS=$(grep -c "^\- \[ \]" .session_todos.txt 2>/dev/null || echo 0)
  if [ $SESSION_TASKS -gt 0 ]; then
    echo "  ✓ Found $SESSION_TASKS tasks in session todo list"
  fi
fi

TODO_TASKS=0
if [ -f TODO.md ]; then
  TODO_TASKS=$(grep -c "^\- \[ \]" TODO.md 2>/dev/null || echo 0)
  if [ $TODO_TASKS -gt 0 ]; then
    echo "  ✓ Found $TODO_TASKS incomplete tasks in TODO.md"
  fi
fi

make checkpoint-update CMD=next-task STEP=2

# ============================================================================
# STAGE 3: Discover Next Task (Step 3)
# ============================================================================

echo ""
echo "========== STAGE 3: Discover Next Task =========="
echo ""

echo "Task discovery workflow:"
echo "  1. Check session todo list first (highest priority)"
echo "  2. If session list is empty, check TODO.md for incomplete tasks"
echo "  3. Present next task to user and wait for feedback"
echo "  4. If both lists are empty, report all tasks complete"
echo ""
echo "Relevant KB articles:"
echo "  - task-verification-before-execution"
echo "  - report-driven-task-planning"
echo "  - tdd-cycle-effort-estimation"
echo "  - user-feedback-as-architecture-gate"
echo ""

make checkpoint-update CMD=next-task STEP=3

# ============================================================================
# Checkpoint Completion
# ============================================================================

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/complete-checkpoint.sh next-task

echo ""
echo "✅ Task discovery workflow complete!"
