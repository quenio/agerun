#!/bin/bash

set -e

# Next Priority Analysis Workflow Script
# This script runs the complete checkpoint-based priority analysis workflow

# Initialize checkpoint tracking or show status if already initialized
./scripts/init-checkpoint.sh next-priority '"Read Context" "Analyze Priorities" "Generate Recommendation"'

# Verify checkpoint tracking is ready
./scripts/require-checkpoint.sh next-priority

# ============================================================================
# STAGE 1: Read Context (Step 1)
# ============================================================================

echo ""
echo "========== STAGE 1: Read Context =========="
echo ""

echo "Reading AGENTS.md and TODO.md..."
echo ""

# Verify necessary files exist
if [ ! -f AGENTS.md ]; then
  echo "❌ ERROR: AGENTS.md not found"
  exit 1
fi

if [ ! -f TODO.md ]; then
  echo "⚠️ WARNING: TODO.md not found - no priorities to analyze"
fi

echo "✅ Context files loaded"

make checkpoint-update CMD=next-priority STEP=1

# ============================================================================
# STAGE 2: Analyze Priorities (Step 2)
# ============================================================================

echo ""
echo "========== STAGE 2: Analyze Priorities =========="
echo ""

echo "Applying systematic analysis protocols..."
echo ""
echo "KB articles for priority analysis:"
echo "  - systematic-task-analysis-protocol"
echo "  - quantitative-priority-setting"
echo "  - task-verification-before-execution"
echo "  - report-driven-task-planning"
echo ""
echo "Analysis framework:"
echo "  1. Identify critical issues (bugs, failures, performance)"
echo "  2. Quantify impact with metrics"
echo "  3. Estimate effort in TDD cycles"
echo "  4. Consider dependencies and prerequisites"
echo "  5. Provide data-driven recommendation"
echo ""

make checkpoint-update CMD=next-priority STEP=2

# ============================================================================
# STAGE 3: Generate Recommendation (Step 3)
# ============================================================================

echo ""
echo "========== STAGE 3: Generate Recommendation =========="
echo ""

echo "Priority analysis complete."
echo ""
echo "⏳ Waiting for your analysis to be presented to the user..."
echo ""
echo "Expected recommendation should include:"
echo "  ✓ Quantitative metrics (error rates, impact, effort)"
echo "  ✓ Justification for the choice"
echo "  ✓ Alternatives considered"
echo "  ✓ Effort estimation in TDD cycles"
echo ""

make checkpoint-update CMD=next-priority STEP=3

# ============================================================================
# Checkpoint Completion
# ============================================================================

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/complete-checkpoint.sh next-priority

echo ""
echo "✅ Priority analysis workflow complete!"
