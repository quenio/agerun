# AgeRun Methodologies Directory

This directory groups loadable methodology sources for fresh executable startup.

Each immediate subdirectory represents one methodology instance source. Put `.method` files inside
that subdirectory using the same filename and instruction format documented in
[methods/README.md](../methods/README.md).

Example layout:

```text
methodologies/
  diagnostic/
    boot-diagnostic-1.0.0.method
  support/
    support-helper-1.0.0.method
```

Available methodology libraries:

- [`coordination/`](coordination/) provides reusable coordination methods for routing,
  supervision, distribution, aggregation, scheduling, synchronization, workflow, conversation,
  and retry.

When `agerun` starts without loading `agerun.methodology`, it first loads the existing
`methods/` directory and then loads each `methodologies/<instance>/` subdirectory that exists.
The existing `methods/` directory behavior is unchanged.

Set `AGERUN_METHODOLOGIES_DIR` to point the executable at an alternate methodologies directory
during tests or local experiments.
