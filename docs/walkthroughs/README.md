# AgeRun Walkthroughs

This directory contains browser-based walkthrough slide decks for explaining specific parts of the AgeRun system in depth.

Each walkthrough is intended to be a focused, reviewable teaching artifact that helps answer questions such as:

- How does a subsystem work end to end?
- Which modules participate in a runtime flow?
- What data moves through the system, and in what order?
- Which source files should be read next for deeper understanding?

## Purpose

Walkthroughs in this directory should:

- explain one focused aspect of the system at a time
- use HTML/JS so they can include diagrams, step-by-step animations, and interactive examples
- stay close to the real codebase by referencing actual files, functions, methods, and runtime behavior
- help a reader build a mental model, not just skim API surface area

## Directory Structure

Each walkthrough should live in its own subdirectory:

```text
docs/
├── index.html
└── walkthroughs/
    ├── index.html
    ├── README.md
    └── <topic-name>/
        ├── index.html
        ├── slides.js
        ├── styles.css        # optional
        └── assets/           # optional
```

## Walkthrough Index

These links open the published walkthrough pages on GitHub Pages.

- Walkthrough landing page: [Walkthroughs](https://quenio.github.io/agerun/walkthroughs/index.html)

| Walkthrough | Topic | Status | Entry Point |
|-------------|-------|--------|-------------|
| agent-lifecycle | How an agent is created, receives messages, executes work, and persists in the runtime | scaffolded | [Agent Lifecycle](https://quenio.github.io/agerun/walkthroughs/agent-lifecycle/index.html) |
| message-processing | How the runtime finds one queued message, executes it, updates scheduling state, and falls back to delegation | scaffolded | [Message Processing](https://quenio.github.io/agerun/walkthroughs/message-processing/index.html) |
| delegation | How the delegation subsystem registers delegates, queues external work, preserves sender metadata, and dispatches one delegate message | scaffolded | [Delegation](https://quenio.github.io/agerun/walkthroughs/delegation/index.html) |
| workflow-method | How the bundled workflow method family boots, validates definitions, evaluates review decisions, and emits visible logs | scaffolded | [Workflow Method](https://quenio.github.io/agerun/walkthroughs/workflow-method/index.html) |

## What to Expect

Most walkthrough decks are designed to help you move from a high-level mental model to concrete runtime details.

A typical walkthrough will usually include:

- a quick statement of what the deck explains
- a structural view of the main runtime parts
- a step-by-step flow for one important behavior
- source-file references for deeper reading
- a suggested next reading path

## How to Use These Walkthroughs

To get the most value from a deck:

1. Start with the published walkthrough link rather than reading the source files directly.
2. Use the slide navigation to skim the structure before reading every slide in detail.
3. Follow the linked source files when you want to verify a claim or go deeper into the implementation.
4. Use the walkthrough index as a map of topics, then jump into the subsystem you care about most.

## Reader Note

If you are browsing these decks as a learner, treat them as guided tours through the repository.

If you are contributing new walkthroughs or editing existing ones, use [`AGENTS.md`](AGENTS.md) for the writer rubric and contribution rules.

## Intended Usage

These walkthroughs are for deep understanding, onboarding, architecture study, and design review. They are not meant to replace the reference documentation in:

- [`README.md`](../../README.md)
- [`modules/README.md`](../../modules/README.md)
- [`methods/README.md`](../../methods/README.md)
- [`SPEC.md`](../../SPEC.md)

Instead, each walkthrough should act as a guided tour through one part of the system and link back to the source documentation where appropriate.
