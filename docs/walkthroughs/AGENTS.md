# AGENTS.md

## Purpose

This directory contains browser-rendered walkthroughs for AgeRun.

These files are teaching artifacts, not just reference docs. They should help a reader build a correct mental model of the system from the real code.

## Audience Split

- `README.md` is for human readers browsing the walkthroughs.
- `AGENTS.md` is for agents and contributors creating or editing walkthrough content.

Do not mix those purposes.

## Writer Rules

### 1. Start from code, not imagination
- Read the referenced source files before editing walkthrough content.
- Do not invent behavior, structure, or terminology.
- If the code is unclear, narrow the claim or mark uncertainty.

### 2. Teach one idea per slide
- Each slide should answer one clear question.
- Prefer progressive disclosure over dense all-in-one explanations.
- If a slide needs too much prose, split it.

### 3. Visuals are required
A walkthrough should not be prose-only.

Include, when relevant:
- a reference architecture view
- a dynamic flow or sequence view
- a state/lifecycle or comparison view

### 4. Prefer lightweight implementation
- Prefer HTML, CSS, and SVG.
- Avoid adding runtime dependencies by default.
- Do not introduce Mermaid, PlantUML, or other renderers unless explicitly justified.

### 5. Preserve visual vocabulary
Use consistent meanings across slides:
- same names
- same colors
- same icon meanings
- same arrow semantics

If `System`, `Agency`, `Agent`, and `Method` appear, keep them visually stable.

### 6. Ground slides in repository structure
- Reference concrete files.
- Prefer explicit file paths over vague phrases like “the runtime code”.
- Public source references should link to GitHub when possible.

### 7. Design for mobile
- Navigation must work on narrow screens.
- Content must remain readable without horizontal scrolling.
- Large diagrams should stack, simplify, or scroll intentionally.

### 8. Prefer strong structure over ornament
Use:
- clear headings
- short labels
- comparison panels
- step flows
- legends where helpful

Avoid:
- large paragraphs
- decorative visuals with no teaching value
- inconsistent terminology

## Recommended Slide Pattern

When applicable, follow this order:

1. What this walkthrough explains
2. Reference architecture / mental model
3. Main lifecycle or flow
4. Deeper step-by-step runtime behavior
5. Persistence / state / lifecycle nuance
6. Code-to-concept mapping
7. Suggested next reading

## Verification Before Completion

Before claiming a walkthrough update is complete, verify:

- desktop rendering works
- mobile navigation works
- visuals remain readable on small screens
- source links work
- claims match the current repository state
- slides still make sense in sequence, not only individually

## Change Discipline

- Keep walkthrough code inspectable and static-host friendly.
- Prefer reusable helpers over slide-specific hacks.
- Do not require a build step unless explicitly approved.
- When adding a new visual pattern, make it reusable for future walkthroughs.
