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

## Visual Design Rubric

Use this rubric for every AgeRun walkthrough so the visuals behave like architecture teaching material instead of ornamental UI.

### 1. Anchor the deck early

Start with one canonical visual within the first one or two slides. That anchor should show the runtime parts the rest of the deck will keep referring to.

For AgeRun, the default anchor is usually:

- System
- Agency
- Agent
- Method
- Message queue or message flow

### 2. Keep one visual question per slide

Each slide should answer one question clearly, such as:

- What are the main runtime parts?
- In what order does one message move?
- What changes during creation?
- What persists across restores?

If a slide needs multiple unrelated visuals, split the slide.

### 3. Reuse the same visual vocabulary

Keep colors, labels, and shapes stable across the whole walkthrough.

Recommended AgeRun defaults:

- `System` → control/runtime color
- `Agency` → coordination color
- `Agent` → stateful entity color
- `Method` → behavior/execution color
- arrows → causality or transition
- pills or callouts → invariants / takeaways

If the same concept changes color or shape between slides, the reader has to relearn the deck.

### 4. Mix three view types

A strong technical walkthrough usually contains all three:

1. **Structure view** — who the parts are
2. **Dynamic view** — what happens over time
3. **Comparison view** — what differs between two states or modes

For AgeRun, that usually means:

- structure: runtime relationship map
- dynamic: one-message sequence or lifecycle flow
- comparison: fresh boot vs restored runtime, queued vs processing, method vs agent

### 5. Visually connect prose to code

When a slide names concrete source files, keep the code references near the diagram they explain.

Good patterns:

- linked source file lists beside the visual
- compact callouts that map a concept to a file
- “read next” blocks that follow the current diagram

### 6. Prefer progressive disclosure over all-in-one diagrams

Do not try to explain the whole runtime in one overloaded picture.

Instead:

- start with a simple anchor
- add one layer of detail at a time
- keep later diagrams visually compatible with the anchor

### 7. Design for desktop and mobile together

All essential visuals must remain understandable on a phone.

That means:

- stacked layouts at narrow widths
- short labels
- no reliance on hover-only interactions
- diagrams that degrade into vertical flows cleanly

### 8. Treat navigation as part of the teaching surface

A walkthrough's navigation is not just chrome. It helps the reader understand scope, sequence, and current position in the story.

That means:

- multi-slide walkthroughs should expose a clear slide index
- desktop should show a visible sidebar or equivalent persistent navigation
- mobile should expose the same structure through an accessible menu
- navigation should show the current slide clearly
- slide titles in navigation should be meaningful enough to scan on their own
- readers should be able to move between slides without relying on hover or hidden gestures

### 9. Use consistent title forms

Slide titles and section titles should follow a stable editorial standard.

That means:

- use capitalized noun phrases
- rewrite verb-phrase titles into noun phrases when possible
- keep pronouns and conjunctions lowercase
- keep articles uppercase
- if a title includes a literal file name or path, preserve its exact spelling

### 10. End with action

The last slide should convert understanding into next steps.

Use:

- a recommended reading order
- linked source files
- obvious follow-up walkthrough topics

## Intended Usage

These walkthroughs are for deep understanding, onboarding, architecture study, and design review. They are not meant to replace the reference documentation in:

- [`README.md`](../../README.md)
- [`modules/README.md`](../../modules/README.md)
- [`methods/README.md`](../../methods/README.md)
- [`SPEC.md`](../../SPEC.md)

Instead, each walkthrough should act as a guided tour through one part of the system and link back to the source documentation where appropriate.
