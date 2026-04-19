# AGENTS.md

## Scope

These instructions apply to all files under `docs/`.

Follow the repository-root `AGENTS.md` by default, except for the docs-only verification override and the page-authoring rules below.

## Docs-Only Verification Override

When **all modified tracked files are under `docs/`**, treat the change as a static documentation/site update.

In that case:

- Do **not** run `make clean build 2>&1`.
- Do **not** run `make check-logs`.
- If any changed file under `docs/` is a Markdown file, run `make check-docs 2>&1` before committing.
- For HTML/CSS/JS-only changes under `docs/`, verify the edited files directly and review the rendered result when practical.

If **any** modified tracked file is outside `docs/`, fall back to the full repository-root pre-commit checklist.

## Site Rules

- `docs/` is the GitHub Pages publishing root for the repository.
- Keep assets static and self-contained.
- Use relative paths between files inside `docs/`.
- Do not place generated build artifacts in `docs/`.
- Keep page code inspectable and static-host friendly.
- Prefer reusable helpers over page-specific hacks.
- Do not require a build step unless explicitly approved.

## Web Page Authoring Rules

These rules apply to any human-facing web page under `docs/`, including landing pages, walkthroughs, and future static reference pages.

### 1. Start from code, not imagination
- Read the referenced source files before editing technical content.
- Do not invent behavior, structure, or terminology.
- If the code is unclear, narrow the claim or mark uncertainty.

### 2. Prefer lightweight implementation
- Prefer HTML, CSS, and SVG.
- Avoid adding runtime dependencies by default.
- Do not introduce extra renderers or frameworks unless explicitly justified.

### 3. Ground pages in repository structure
- Reference concrete files.
- Prefer explicit file paths over vague phrases like “the runtime code”.
- Public source references should link to GitHub when possible.
- Keep assets local to the relevant `docs/` subtree and use relative paths inside the page.
- Do not depend on generated files in `bin/`.

### 4. Preserve visual vocabulary
Use consistent meanings across related pages:
- same names
- same colors
- same icon meanings
- same arrow semantics

If a page shows AgeRun concepts such as the system, agency, agent, method, or message flow, keep them visually stable across the site.

### 5. Treat navigation as part of the page
- Multi-section pages should provide clear navigation.
- Desktop navigation should remain visible or otherwise obvious.
- Narrow-screen navigation should stay accessible.
- Navigation should clearly indicate the current section when applicable.
- Navigation must not rely on hover-only interaction or hidden gestures.

### 6. Design for mobile
- Content must remain readable on narrow screens.
- Avoid accidental horizontal scrolling.
- Large diagrams should stack, simplify, or scroll intentionally.

### 7. Prefer full-viewport shells for app-like pages
When a page has persistent navigation, persistent controls, or a multi-pane workspace, treat it like an app shell rather than a centered document card.

- Use the available viewport width and height.
- Keep outer padding minimal.
- Keep persistent top and bottom bars visible.
- Make navigation and primary content areas independently scrollable when both can grow.
- Avoid layouts where the entire page scroll hides the main controls.
- On narrow screens, preserve the same structure with an accessible overlay or stacked equivalent.

### 8. Prefer readable code references in prose
- In tutorial or explanatory sentences, prefer module-and-action phrasing over raw long function names.
- Use italic human-readable operation names when that keeps prose easier to scan.
- Reserve exact function names for code snippets, source-file references, or compact callouts where the raw symbol matters.
- If a raw symbol is necessary, keep it visually separate from the main sentence so the prose remains readable.

### 9. Use consistent title forms
- Use capitalized noun phrases for page titles, section titles, and navigation labels when possible.
- Rewrite verb-phrase titles into noun phrases when possible.
- Keep pronouns and conjunctions lowercase.
- Keep articles uppercase.
- Preserve literal file names and paths exactly when they appear in titles.

### 10. Prefer strong structure over ornament
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

## Verification Before Completion

Before claiming a `docs/` page update is complete, verify:

- desktop rendering works
- narrow-screen rendering works
- navigation remains usable where the page has navigation
- persistent bars remain visible where the page uses an app-like shell
- multi-pane shells use independent scrolling where appropriate
- links work
- claims match the current repository state
- technical prose stays readable and does not overuse raw long symbols

## Walkthrough Subtree

For files under `docs/walkthroughs/`, also follow `docs/walkthroughs/AGENTS.md` for walkthrough-specific slide and teaching rules.
