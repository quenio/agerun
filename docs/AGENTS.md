# AGENTS.md

## Scope

These instructions apply to all files under `docs/`.

Follow the repository-root `AGENTS.md` by default, except for the docs-only verification override and the page-authoring rules below.

## Docs-Only Verification Override

When **all modified tracked files are under `docs/`**, or the only modified tracked file outside `docs/`
is `CHANGELOG.md`, treat the change as a docs-site update.

In that case:

- Do **not** run `make clean build 2>&1`.
- Do **not** run `make check-logs`.
- If any changed file under `docs/` is a Markdown file, run `make check-docs 2>&1` before committing.
- For HTML/CSS/JS-only changes under `docs/`, verify the edited files directly and review the rendered result when practical.

If any modified tracked file outside `docs/` is something other than `CHANGELOG.md`, fall back to the
full repository-root pre-commit checklist.

## Site Rules

- `docs/` is the GitHub Pages publishing root for the repository.
- Keep assets self-contained and GitHub Pages friendly.
- Use relative paths between files inside `docs/`.
- Do not place generated build artifacts in `docs/`.
- Keep page code inspectable and easy to maintain.
- Prefer reusable helpers over page-specific hacks.
- Do not require a build step unless explicitly approved.
- In shared docs CSS, avoid broad element selectors or generic adjacency rules that can accidentally hit
  app-shell pages. In particular, avoid global layout rules like `main { ... }` and generic sibling
  spacing like `.panel + .panel { ... }` unless they are tightly scoped.

## Web Page Authoring Rules

These rules apply to any human-facing web page under `docs/`, including landing pages, walkthroughs, and future published reference pages.

### 1. Start from code, not imagination
- Read the referenced source files before editing technical content.
- Do not invent behavior, structure, or terminology.
- If the code is unclear, narrow the claim or mark uncertainty.

### 2. Prefer lightweight implementation
- Prefer HTML, CSS, and SVG.
- Avoid adding runtime dependencies by default.
- Do not introduce extra renderers or frameworks unless explicitly justified.

### 3. Ground pages in repository truth
- Base technical claims on the real codebase and current repository state.
- Reference concrete files when that helps the reader verify or understand the page.
- Do not force implementation details or file paths into audience-facing prose when they do not help.
- Public source references should link to GitHub when they add value.
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
- If navigation labels and content labels describe the same sections, keep their typography and color
  relationships visually aligned.

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
- Align pane edges through the shell layout first. Do not fake panel alignment by inflating inner
  padding when the real issue is panel placement.
- If a pane should visibly reach the layout edge, ensure the grid item can actually shrink inside the
  shell (`min-width: 0` where needed) and does not inherit document-style width rules.
- When panes scroll independently, provide a visible cue that more content exists above or below, and
  keep that cue pinned to the pane edge rather than letting it move with the scrolled content.

### 8. Keep docs index pages consistent across subfolders
- Treat every `index.html` page under `docs/` and its subfolders as part of one family.
- Subfolder index pages should follow the same shell vocabulary and visual rhythm as `docs/index.html`
  unless a clear user-approved reason requires a different pattern.
- Reuse the shared top bar, sidebar, content pane, footer bar, cards, and chip-link styles from
  `docs/site.css` instead of inventing one-off index layouts.
- Keep the same hierarchy expectations on index pages: concise eyebrow if used, one clear page title,
  short supporting copy, visible section navigation, and a quiet footer with only useful links.
- If one docs index page gets a navigation, spacing, footer, copy, or card-treatment improvement that
  should generalize, apply or document the same guidance for other subfolder index pages too.

### 9. Prefer readable code references in prose
- In tutorial or explanatory sentences, prefer module-and-action phrasing over raw long function names.
- Use italic human-readable operation names when that keeps prose easier to scan.
- Reserve exact function names for code snippets, source-file references, or compact callouts where the raw symbol matters.
- If a raw symbol is necessary, keep it visually separate from the main sentence so the prose remains readable.

### 10. Use consistent title forms
- Use capitalized noun phrases for page titles, section titles, and navigation labels when possible.
- Rewrite verb-phrase titles into noun phrases when possible.
- Keep pronouns and conjunctions lowercase.
- Keep articles uppercase.
- Preserve literal file names and paths exactly when they appear in titles.
- In top bars and section bodies, keep only one necessary visible title layer. Remove redundant eyebrow
  labels or repeated in-section headings when the page already establishes the same title clearly.

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
- repeated status strips or explanatory chrome text that does not help the reader act

### 12. Keep published-page copy audience-first
- Prefer educational, product-facing language over implementation or publishing-mechanism wording.
- Do not explain that a page is "static", "browser-rendered", or otherwise implementation-shaped unless
  that fact helps the reader.
- Do not expose repository structure details in overview copy when they do not help the audience.
- For published pages that link back into the repository, use real GitHub URLs when the target file is
  outside the published `docs/` site root.

## How to Test and Verify Pages Under `docs/`

Use a concrete verification pass for every HTML/CSS/JS page change under `docs/`. Do not rely on static
inspection alone for layout or interaction changes.

### 1. Validate the edited assets first
- If any edited file under `docs/` is Markdown, run `make check-docs 2>&1`.
- Do not escalate docs-page verification to `make clean build 2>&1` or `make check-logs` just because
  the commit also updates `CHANGELOG.md`.
- If you edited JavaScript, run `node -c <path-to-file.js>` on each changed JS file before claiming the
  page works.
- If you changed shared assets such as `docs/site.css`, `docs/site.js`, `docs/walkthroughs/deck.css`, or
  `docs/walkthroughs/deck.js`, verify at least one representative page for every page family affected.

### 2. Render the real page locally
Use the real page file with a `file://` URL and capture a fresh screenshot instead of reasoning from the
source alone.

Desktop example:
```bash
npx --yes playwright screenshot --device="Desktop Chrome" --wait-for-timeout=300 \
  file:///ABSOLUTE/PATH/TO/docs/index.html /tmp/docs-index-check.png
```

Narrow-screen example:
```bash
npx --yes playwright screenshot --device="iPhone 13" --wait-for-timeout=300 \
  file:///ABSOLUTE/PATH/TO/docs/walkthroughs/index.html /tmp/docs-walkthroughs-mobile-check.png
```

Use absolute file URLs that point at the actual page being edited.

### 3. Verify the rendered result, not just command success
Review the fresh screenshot and confirm the specific change is visible.

At minimum, check:
- desktop rendering works
- narrow-screen rendering works
- navigation remains usable where the page has navigation
- persistent bars remain visible where the page uses an app-like shell
- multi-pane shells use independent scrolling where appropriate
- pane edges align as intended in app-shell pages
- scroll cues appear only when more content exists above or below and stay pinned to pane edges
- links work
- claims match the current repository state
- technical prose stays readable and does not overuse raw long symbols
- subfolder index pages under `docs/` follow the shared index-page shell and style guidance when touched

### 4. Test interactions when the page has them
If the page has interactive behavior, verify the interaction itself, not just the first paint.

Examples:
- walkthrough decks: previous/next controls, keyboard navigation, sidebar open/close, back links
- app-shell pages: sidebar navigation, independently scrollable panes, top/bottom bars staying visible
- shared JS changes: verify multiple pages that consume the shared script

Capture fresh screenshots after the interaction when the bug or change is interaction-sensitive.

### 5. Re-check links from the published-page perspective
- Relative links are correct only for files that are actually published under `docs/`.
- For repository files outside the published site root, use GitHub URLs instead of broken relative links
  like `../README.md` from published pages.
- When link targets change, run `make check-docs 2>&1` even for docs-site work that is otherwise HTML/CSS/JS.

## Verification Before Completion

Before claiming a `docs/` page update is complete, verify:

- the concrete page-verification workflow above was followed
- visual fixes are confirmed with a fresh rendered screenshot when the change is layout-sensitive

## Walkthrough Subtree

For files under `docs/walkthroughs/`, also follow `docs/walkthroughs/AGENTS.md` for walkthrough-specific slide and teaching rules.
