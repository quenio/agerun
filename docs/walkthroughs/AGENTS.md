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

If the walkthrough shows the system, agency, agent, and method concepts, keep them visually stable.

### 6. Ground slides in repository structure
- Reference concrete files.
- Prefer explicit file paths over vague phrases like “the runtime code”.
- Public source references should link to GitHub when possible.
- Keep assets local to the walkthrough directory and use relative paths inside the deck.
- Do not depend on generated files in `bin/`.

### 7. Treat navigation as part of the walkthrough
- For multi-slide walkthroughs, provide a clear slide index.
- Desktop should expose a visible sidebar or equivalent persistent navigation.
- Mobile should expose the same slide structure through an accessible menu.
- Navigation should clearly indicate the current slide.
- Slide titles should be meaningful enough to scan from the navigation alone.
- Navigation must not rely on hover-only interaction or hidden gestures.

### 8. Design for mobile
- Navigation must work on narrow screens.
- Content must remain readable without horizontal scrolling.
- Large diagrams should stack, simplify, or scroll intentionally.

### 9. Prefer readable code references in prose
- In tutorial sentences, prefer module-and-action phrasing over raw long function names.
- Use italic human-readable operation names, for example “the *process next message* operation of the system module” instead of raw symbols like `ar_system__process_next_message(...)`.
- Reserve exact function names for code snippets, source-file references, or compact callouts where the raw symbol matters.
- If a raw symbol is necessary, keep it visually separate from the main sentence so the prose remains easy to scan.

### 10. Use consistent title forms
- Use capitalized noun phrases for slide titles and section titles.
- Rewrite verb-phrase titles into noun phrases when possible.
- Keep pronouns and conjunctions lowercase.
- Keep articles uppercase.
- Preserve literal file names and paths exactly when they appear in titles.

### 11. Prefer strong structure over ornament
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

## Visual Teaching Rubric

### 1. Anchor the deck early
- Start with one canonical visual within the first one or two slides.
- That anchor should show the runtime parts the rest of the deck will keep referring to.
- For AgeRun, the default anchor is usually the system, the agency, an agent, a method, and a message queue or message flow.

### 2. Keep one visual question per slide
- Each slide should answer one question clearly.
- If a slide needs multiple unrelated visuals, split the slide.

### 3. Mix three view types
A strong technical walkthrough usually contains all three when relevant:
- structure view — who the parts are
- dynamic view — what happens over time
- comparison view — what differs between two states or modes

### 4. Visually connect prose to code
When a slide names concrete source files, keep the code references near the diagram they explain.

Good patterns:
- linked source file lists beside the visual
- compact callouts that map a concept to a file
- “read next” blocks that follow the current diagram

### 5. Prefer progressive disclosure over overloaded diagrams
- Do not try to explain the whole runtime in one overloaded picture.
- Start with a simple anchor.
- Add one layer of detail at a time.
- Keep later diagrams visually compatible with the anchor.

### 6. End with action
The last slide should convert understanding into next steps.

Use:
- a recommended reading order
- linked source files
- obvious follow-up walkthrough topics

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
- desktop navigation remains visible and usable
- mobile navigation works
- navigation reflects the slide structure and current position clearly
- slide titles and section titles follow the title standard consistently
- tutorial prose avoids raw long function names where a module/action phrase is clearer
- visuals remain readable on small screens
- source links work
- claims match the current repository state
- slides still make sense in sequence, not only individually

## Publishing Conventions

- Walkthrough index entries should use absolute GitHub Pages URLs.
- For this repository, published walkthrough URLs are rooted at `https://quenio.github.io/agerun/`.
- Use the walkthrough's human-readable title as the index link caption, not a raw file path.

## Change Discipline

- Keep walkthrough code inspectable and static-host friendly.
- Prefer reusable helpers over slide-specific hacks.
- Do not require a build step unless explicitly approved.
- When adding a new visual pattern, make it reusable for future walkthroughs.
