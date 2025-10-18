Read AGENTS.md in order to prepare yourself for this new session. Then, suggest the next priority based on the TODO.md file.

## MANDATORY FIRST STEP - KB Consultation

Before analyzing priorities, you MUST ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search the KB index: `grep "priority\|task\|analysis\|systematic" kb/README.md`
2. Read relevant articles, especially:
   - systematic-task-analysis-protocol ([details](../../../kb/systematic-task-analysis-protocol.md))
   - quantitative-priority-setting ([details](../../../kb/quantitative-priority-setting.md))
   - task-verification-before-execution ([details](../../../kb/task-verification-before-execution.md))
   - report-driven-task-planning ([details](../../../kb/report-driven-task-planning.md))
3. Show your KB searches and findings in the conversation
4. Apply the protocols from these articles

Only after completing KB consultation should you proceed to analyze TODO.md.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/next-priority` workflow is already in progress:

```bash
make checkpoint-status CMD=next_priority VERBOSE=--verbose
# Resume: make checkpoint-update CMD=next_priority STEP=N
# Or reset: make checkpoint-cleanup CMD=next_priority && make checkpoint-init CMD=next_priority STEPS='"Prepare" "Execute" "Verify"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/next_priority_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=next_priority STEPS='"Prepare" "Execute" "Verify"'
else
  make checkpoint-status CMD=next_priority
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/next_priority_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Next Priority
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the next priority process
make checkpoint-init CMD=next_priority STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: next_priority (3 steps)
📁 Tracking: /tmp/next_priority_progress.txt
→ Run: make checkpoint-update CMD=next_priority STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=next_priority
```

**Expected output (example at 33% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues


## Expected Behavior

#### [CHECKPOINT START - EXECUTION]


### Priority Analysis Output
```
Reading AGENTS.md...
Analyzing TODO.md for priorities...

PRIORITY RECOMMENDATION: Parser Error Logging Enhancement

QUANTITATIVE JUSTIFICATION:
• Silent failure rate: 97.6% (41 of 42 error conditions)
• Debugging time impact: 50-70% increase
• Affected parsers: 11 modules
• Completion: 18% (2 of 11 complete)

WHY THIS PRIORITY:
1. Highest measurable impact (97.6% failure rate)
2. Clear scope (9 parsers × 2-3 cycles each)
3. Low risk (logging doesn't change behavior)
4. Foundation work (helps all future development)

ALTERNATIVES CONSIDERED:
- Memory leak fixes: Only 3 known leaks (low impact)
- Documentation updates: No quantitative urgency
- New features: Should fix foundation first

Estimated effort: 18-27 TDD cycles
Timeline: 2-3 sessions

[Waits for user verification]
```

### After User Approval
```
Creating implementation plan...

TODO LIST:
1. [pending] Add error logging to ar_build_parser
   - NULL parameter checks (1 cycle)
   - Parse failure logging (1 cycle)
   - Documentation update (0.5 cycle)
   
2. [pending] Add error logging to ar_spawn_parser
   - NULL parameter checks (1 cycle)
   - Parse failure logging (1 cycle)
   - Documentation update (0.5 cycle)

... (7 more parsers)

Total: 9 parsers × 2.5 cycles = 22.5 TDD cycles

Ready to begin implementation?
[Waits for plan approval]
```

### Low Priority Scenario
```
Reading AGENTS.md...
Analyzing TODO.md for priorities...

ANALYSIS COMPLETE:
✅ Critical issues: None found
✅ High-impact bugs: None pending
✅ Performance issues: None identified

SUGGESTED PRIORITY: Code Quality Improvements

QUANTITATIVE METRICS:
• Code duplication: 3 instances found
• Test coverage: 92% (could reach 95%)
• Documentation gaps: 5 modules need updates

This is a LOW-URGENCY priority suitable for:
- Learning the codebase
- Gradual improvement
- Between major features

Estimated effort: 5-8 TDD cycles

[Waits for user direction]
```


#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh next-priority
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 next-priority: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```


#### [CHECKPOINT END - EXECUTION]

## Key Points

When determining priority, use quantitative metrics where available ([details](../../../kb/quantitative-priority-setting.md)):
- Error rates (e.g., "97.6% silent failures")
- Code duplication metrics
- Performance impacts
- Time/cost savings potential

Explain why you made that priority choice using specific data. Wait for my verification and suggestions.

If I approved your choice, define a plan in the todos list in order to execute it. Estimate effort in TDD cycles ([details](../../../kb/tdd-cycle-effort-estimation.md)). Wait for my approval of the plan, or for my adjustment to the plan.