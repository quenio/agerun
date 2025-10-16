# Claude Code Configuration

**When you don't understand something about the project**: Check [AGENTS.md](AGENTS.md) first.

See [AGENTS.md](AGENTS.md) for detailed information about the project structure, agents, development guidelines, coding standards, and all critical patterns.

## Critical Checkpoint Workflow Discipline

⚠️ **MANDATORY REQUIREMENTS for All Checkpoint Commands**:

### Never Skip Steps
- **ALL steps must execute** - No skipping, no batching completions
- **Each step requires actual work** - Never mark a step complete without performing the required action
- **Sequential execution is mandatory** - Complete Step N before moving to Step N+1
- **Mark as in_progress before work** - Update todo list before executing each step
- **Mark as completed after work** - Only after actual work is done, verify and then mark complete

### Gate Checks Are Mandatory
- **Run all required gates** - Do not skip or bypass gate checks at stage boundaries
- **Verify gate passes** - Confirm gate output shows ✅ before proceeding to next stage
- **Understand gate purpose** - Gates validate quality and completeness between workflow stages
- **Document gate results** - Include gate verification in commit messages when gates are required

### Work Verification Pattern
- **Verify before marking complete** - Check that actual output exists before updating checkpoint
- **Output verification** - Ensure commands executed, tests passed, or files modified
- **Quality gates** - Run gate checks at required points to validate work quality
- **No hypothetical completions** - Only mark steps complete when actual deliverables exist

### Conditional Workflows
- **Distinguish contexts** - Conditional step skipping is ONLY for resuming in-progress workflows
- **Fresh initialization** - On first run, execute all steps sequentially regardless of conditional logic
- **Document conditions** - Make conditions explicit in command documentation
- **Gate-based branching** - Use gates to determine conditional flow, not arbitrary logic