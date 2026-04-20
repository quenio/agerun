# AGENTS.md

## Purpose

This directory contains browser-rendered walkthroughs for AgeRun.

These files are teaching artifacts, not just reference docs. They should help a reader build a correct mental model of the system from the real code.

## Audience Split

- `README.md` is for human readers browsing the walkthroughs.
- `AGENTS.md` is for agents and contributors creating or editing walkthrough content.

Do not mix those purposes.

## Writer Rules

Follow `docs/AGENTS.md` for the shared `docs/` web-page rules. The rules below are specific to walkthrough decks.

### 1. Teach one idea per slide
- Each slide should answer one clear question.
- Prefer progressive disclosure over dense all-in-one explanations.
- If a slide needs too much prose, split it.

### 2. Visuals are required
A walkthrough should not be prose-only.

Include, when relevant:
- a reference architecture view
- a dynamic flow or sequence view
- a state/lifecycle or comparison view

### 3. Walkthrough navigation must expose slide structure
- For multi-slide walkthroughs, provide a clear slide index.
- Desktop should expose a visible sidebar or equivalent persistent navigation.
- Mobile should expose the same slide structure through an accessible menu.
- Navigation should clearly indicate the current slide.
- Slide titles should be meaningful enough to scan from the navigation alone.
- For deck shells, the navigation sidebar should extend to the bottom of the main shell rather than
  stopping above the footer controls.

### 4. Walkthrough deck shell controls should read as one system
- Previous/next controls belong under the slide pane, not under the navigation sidebar.
- On desktop, keep the previous button on the left, the keyboard hint centered, and the next button on
  the right.
- Right-align the next-button label so the forward action reads naturally at the far edge.
- On narrow screens, it is acceptable to fall back to the stacked mobile control layout.

## Visual Teaching Rubric

### 1. Anchor the deck early

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

### 7. Keep walkthrough index pages quiet
For `docs/walkthroughs/index.html` and similar landing pages:
- use the main title without an extra eyebrow label in the top bar
- avoid repeating large in-section titles when the section label already does the job
- remove explanatory footer text when the bottom-bar links are enough
- keep overview copy educational instead of explaining the publishing mechanism

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

- the shared `docs/AGENTS.md` page checks pass
- desktop navigation remains visible and usable
- mobile navigation works
- navigation reflects the slide structure and current position clearly
- slide titles are meaningful when scanned from the navigation alone
- the sidebar reaches the bottom of the shell where the deck uses the desktop two-pane layout
- the footer controls align only with the slide pane where the deck uses desktop previous/next controls
- pane scroll cues stay pinned to the pane edge rather than moving with scrolled content
- slides still make sense in sequence, not only individually

## Publishing Conventions

- Walkthrough index entries should use absolute GitHub Pages URLs.
- For this repository, published walkthrough URLs are rooted at `https://quenio.github.io/agerun/`.
- Use the walkthrough's human-readable title as the index link caption, not a raw file path.

## Change Discipline

- When adding a new visual pattern, make it reusable for future walkthroughs.
