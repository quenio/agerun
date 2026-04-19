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
walkthroughs/
├── README.md
└── <topic-name>/
    ├── index.html
    ├── slides.js
    ├── styles.css        # optional
    └── assets/           # optional
```

### Naming

Use kebab-case directory names that describe the topic clearly.

Examples:

- `agent-lifecycle/`
- `method-loading/`
- `message-processing/`
- `shell-runtime-flow/`

## Walkthrough Index

The first walkthrough scaffold has been added below.

Add new entries here as decks are created.

Use absolute GitHub Pages URLs for walkthrough entry points so browsers can render the HTML/JS deck directly.

| Walkthrough | Topic | Status | Entry Point |
|-------------|-------|--------|-------------|
| agent-lifecycle | How an agent is created, receives messages, executes work, and persists in the runtime | scaffolded | [Agent Lifecycle](https://quenio.github.io/agerun/walkthroughs/agent-lifecycle/index.html) |

## Authoring Guidelines

When creating a walkthrough:

1. Keep the scope narrow and teach one major concept well.
2. Prefer real AgeRun file paths and symbols over hypothetical examples.
3. Show sequence and causality clearly: what starts first, what happens next, what state changes, and what outputs are produced.
4. Use diagrams, timelines, and message-flow views when they make the system easier to understand.
5. Keep assets local to the walkthrough directory and use relative paths.
6. Use an absolute GitHub Pages URL for each walkthrough index entry so the deck can be opened directly in a browser from the repository README.
7. For this repository, walkthrough deck URLs should be rooted at `https://quenio.github.io/agerun/`.
8. Use the walkthrough's human-readable title as the link caption, not the raw file path.
9. Do not depend on generated files in `bin/`.
10. When referencing code, point readers toward the relevant files in `modules/`, `methods/`, `specs/`, or root documentation.

## Intended Usage

These walkthroughs are for deep understanding, onboarding, architecture study, and design review. They are not meant to replace the reference documentation in:

- [`README.md`](../README.md)
- [`modules/README.md`](../modules/README.md)
- [`methods/README.md`](../methods/README.md)
- [`SPEC.md`](../SPEC.md)

Instead, each walkthrough should act as a guided tour through one part of the system and link back to the source documentation where appropriate.
