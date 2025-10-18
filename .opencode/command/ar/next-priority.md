Read AGENTS.md in order to prepare yourself for this new session. Then, suggest the next priority based on the TODO.md file.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Initialization

This command requires checkpoint tracking to ensure systematic workflow execution.

### Initialize Tracking

```bash
./scripts/init-checkpoint.sh next-priority '"Read Context" "Analyze Priorities" "Generate Recommendation"'
```

**Expected output:**
```
📍 Starting: next-priority (3 steps)
📁 Tracking: /tmp/next-priority-progress.txt
→ Run: make checkpoint-update CMD=next-priority STEP=1
```

### Check Progress

```bash
make checkpoint-status CMD=next-priority
```

**Expected output (example at 33% completion):**
```
📈 next-priority: 1/3 steps (33%)
   [███░░░░░░░░░░░░░░░░░░] 33%
→ Next: make checkpoint-update CMD=next-priority STEP=2
```

## Checkpoint Tracking

This command uses checkpoint tracking with 3 sequential steps to ensure systematic priority analysis:

1. **Read Context** - Understand project structure and review TODO.md
2. **Analyze Priorities** - Apply systematic analysis protocols using quantitative metrics
3. **Generate Recommendation** - Provide priority suggestion with justification

Each step requires completion before moving to the next. Use `make checkpoint-status CMD=next-priority` to check progress at any time.

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

# Priority Analysis Workflow

#### [CHECKPOINT START]

## Step 1: Read Context

#### [CHECKPOINT STEP 1]

**What this step does:**
- Reads AGENTS.md to understand the project context
- Checks TODO.md for incomplete tasks to analyze
- Verifies necessary files exist

**What you should do:**
```bash
# After initializing checkpoints, read the context from:
# - AGENTS.md: Project structure, guidelines, agent responsibilities
# - TODO.md: List of incomplete tasks marked with [ ]

# Verify both files exist and contain the expected content
ls -la AGENTS.md TODO.md
```

**Expected files:**
- `AGENTS.md`: Project documentation and guidelines
- `TODO.md`: Task list with incomplete items marked `- [ ]`

**Next action:**
```bash
./scripts/update-checkpoint.sh next-priority 1
```

## Step 2: Analyze Priorities

#### [CHECKPOINT STEP 2]

**What this step does:**
- Applies systematic analysis protocols from KB articles
- Evaluates tasks using quantitative metrics
- Determines impact, effort, and risk for each task

**Analysis framework:**
1. **Identify critical issues** - bugs, failures, performance problems
2. **Quantify impact** - error rates, affected users/modules, time savings
3. **Estimate effort** - TDD cycles required to complete
4. **Consider dependencies** - prerequisites and blocking tasks
5. **Evaluate risk** - complexity, testing difficulty, potential side effects

**What you should do:**

Apply the systematic analysis from KB articles:
- Review each incomplete task in TODO.md
- Gather quantitative metrics (if available in codebase)
- Evaluate each task using the analysis framework
- Rank tasks by impact/effort ratio

**Example analysis:**
```
Task: "Add error logging to parsers"
- Impact: 97.6% of error conditions currently silent
- Effort: 18-27 TDD cycles (9 parsers × 2-3 cycles)
- Risk: Low (logging doesn't change behavior)
- Dependencies: None
- Rank: HIGH (high impact, low risk)

Task: "Fix memory leaks"
- Impact: Only 3 known leaks in non-critical paths
- Effort: 8-12 TDD cycles
- Risk: Medium (memory management changes)
- Dependencies: Some depend on refactoring
- Rank: MEDIUM (lower impact than logging)
```

**Next action:**
```bash
./scripts/update-checkpoint.sh next-priority 2
```

## Step 3: Generate Recommendation

#### [CHECKPOINT STEP 3]

**What this step does:**
- Summarizes the analysis results
- Provides quantitative justification for priority choice
- Prepares recommendation for user feedback
- Waits for user verification

**What you should do:**

Based on your analysis, provide a recommendation with:
1. **Selected priority** - Which task should be done first
2. **Quantitative justification** - Metrics supporting the choice
3. **Alternatives considered** - Why other tasks ranked lower
4. **Effort estimation** - TDD cycles needed for the selected task
5. **Next steps** - What happens after user approves

**Recommendation format:**
```
PRIORITY RECOMMENDATION: [Task Name]

QUANTITATIVE JUSTIFICATION:
• [Metric 1]: [value and context]
• [Metric 2]: [value and context]
• [Metric 3]: [value and context]

WHY THIS PRIORITY:
1. [Reason 1 with specific data]
2. [Reason 2 with specific data]
3. [Reason 3 with specific data]

ALTERNATIVES CONSIDERED:
- [Task A]: [Why it ranked lower]
- [Task B]: [Why it ranked lower]

Estimated effort: [X-Y] TDD cycles
Timeline: [estimate]

[Waits for user verification]
```

**Example output:**
```
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

**After user approval:**

If the user accepts your recommendation, prepare for implementation:
```bash
# Create implementation plan
/ar:create-plan "Task Name" "Additional context if needed"

# Or move to next-task to discover other work items
/ar:next-task
```

**Next action:**
```bash
./scripts/update-checkpoint.sh next-priority 3
```

## Complete the Workflow

#### [CHECKPOINT END]

#### [CHECKPOINT GATE]

When all steps are complete, verify the workflow:

```bash
./scripts/complete-checkpoint.sh next-priority
```

#### [CHECKPOINT COMPLETE]

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 next-priority: 3/3 steps (100%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```

## Minimum Requirements

**MANDATORY for successful priority analysis:**
- [ ] KB consultation completed before analysis
- [ ] AGENTS.md read to understand project context
- [ ] TODO.md reviewed for incomplete tasks
- [ ] Quantitative metrics gathered for each task
- [ ] Priority recommendation provided with justification
- [ ] Alternatives considered and documented
- [ ] Effort estimation provided in TDD cycles
- [ ] User verification obtained before proceeding

### Progress Tracking

Monitor your progress through the 3-step workflow:

```bash
# Initialize checkpoint tracking
./scripts/init-checkpoint.sh next-priority '"Read Context" "Analyze Priorities" "Generate Recommendation"'

# Check current checkpoint status
make checkpoint-status CMD=next-priority VERBOSE=--verbose

# Update to next step (after completing current step)
make checkpoint-update CMD=next-priority STEP=N

# Complete the workflow
./scripts/complete-checkpoint.sh next-priority
```

## Key Points

When determining priority, use quantitative metrics where available ([details](../../../kb/quantitative-priority-setting.md)):
- Error rates (e.g., "97.6% silent failures")
- Code duplication metrics
- Performance impacts
- Time/cost savings potential

Explain why you made that priority choice using specific data. Wait for my verification and suggestions.

If I approved your choice, define a plan in the todos list in order to execute it. Estimate effort in TDD cycles ([details](../../../kb/tdd-cycle-effort-estimation.md)). Wait for my approval of the plan, or for my adjustment to the plan.
