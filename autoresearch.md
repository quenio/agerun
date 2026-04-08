# Autoresearch: System-Wide Integration Testing & Performance

## Objective
Optimize the end-to-end performance of the AgeRun agent system under a high-stress integration workload. The goal is to maximize throughput (minimize processing time) while maintaining absolute memory safety and correctness across complex agent interactions.

## Metrics
- **Primary**: `total_µs` (µs, lower is better) — Total time to process a predefined stress-test workload of 100 agents in a ring processing 1000 messages.
- **Secondary**:
  - `memory_bytes` (bytes) — Peak memory usage during the workload.
  - `msg_per_sec` (msg/s) — Number of messages processed per second.

## How to Run
`make ar_integration_performance_tests` — Builds and runs `modules/ar_integration_performance_tests.c` only, and outputs `METRIC total_µs=...` for faster turnaround.

## Files in Scope
- `modules/ar_system.c/h` - Core system loop and message processing.
- `modules/ar_agency.c/h` - Agent management and message routing.
- `modules/ar_delegation.c/h` - Delegate system integration.
- `modules/ar_frame.c/h` - Execution context and frame management.
- `modules/ar_method_evaluator.c/h` (and other evaluators) - Execution efficiency.
- `modules/ar_data.c/h` - Data type efficiency and ownership.

## Off Limits
- Do not change the defined benchmark workload to "cheat" the metrics.
- Do not disable memory tracking or sanitizers in `autoresearch.checks.sh`.

## Constraints
- All existing tests in `modules/*.c` must pass.
- Zero memory leaks allowed (verified by `make run-tests`).
- Must maintain ABI compatibility for C/Zig modules.

## What's Been Tried
- (To be updated as experiments progress)
