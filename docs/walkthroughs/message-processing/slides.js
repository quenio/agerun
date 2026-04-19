const repositoryBaseUrl = "https://github.com/quenio/agerun/blob/main/";
const repositoryPathPattern = /\b(?:README\.md|SPEC\.md|CHANGELOG\.md|TODO\.md|CONCEPTS\.md|MMM\.md|CLAUDE\.md|AGENTS\.md|(?:modules|methods|specs|workflows|scripts|docs|kb|models|reports|plans|docker|llama-cpp)\/[A-Za-z0-9._/-]+\.[A-Za-z0-9._-]+)\b/g;
const delegationWalkthroughUrl = "https://quenio.github.io/agerun/walkthroughs/delegation/index.html";

const slides = [
    {
        title: "Message Processing",
        subtitle: "How the <em>process next message</em> operation of the system module finds one pending unit of work, executes it, and decides what happens next.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Walkthrough Scope</h3>
                    <ul>
                        <li>The public processing contract in the system API</li>
                        <li>How messages move into and out of agent queues</li>
                        <li>How the runtime chooses which agent to process next</li>
                        <li>How interpreter execution and delegate fallback fit into the loop</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Primary Source Files</h3>
                    <div class="path-list">modules/ar_system.h
modules/ar_system.c
modules/ar_agency.h
modules/ar_agency.c
modules/ar_agent.h
modules/ar_agent.c
modules/ar_interpreter.h
modules/ar_interpreter.c
modules/ar_delegation.h
modules/ar_delegation.c
README.md</div>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Processing Surface</h3>
                <div class="reference-architecture">
                    <div class="legend-row">
                        <span class="legend-chip legend-system">System</span>
                        <span class="legend-chip legend-agency">Agency</span>
                        <span class="legend-chip legend-agent">Agent</span>
                        <span class="legend-chip legend-method">Method</span>
                        <span class="legend-chip legend-flow">Message Flow</span>
                    </div>
                    <div class="anchor-stack">
                        <div class="anchor-top">
                            <div class="map-node map-node-system">
                                <strong>System</strong>
                                <span>scans queues and runs one step</span>
                            </div>
                        </div>
                        <div class="anchor-middle">
                            <div class="map-node map-node-agency">
                                <strong>Agency</strong>
                                <span>finds agents and pops queued messages</span>
                            </div>
                            <div class="anchor-flow">scan queues → execute method → update cursor</div>
                            <div class="map-node map-node-agent">
                                <strong>Agent</strong>
                                <span>owns memory and FIFO queue</span>
                            </div>
                            <div class="map-node map-node-method">
                                <strong>Method</strong>
                                <span>runs through the interpreter</span>
                            </div>
                        </div>
                        <div class="system-map-footer">
                            <span class="map-pill">If no agent queue has work, the system falls back to delegation.</span>
                        </div>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Processing Contract",
        subtitle: "The system API exposes <em>process next message</em> and <em>process all messages</em> as separate operations.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Public API Surface</h3>
                    <ul>
                        <li>The system module's <em>process next message</em> operation processes at most one pending unit of work.</li>
                        <li>The system module's <em>process all messages</em> operation keeps looping until the runtime reports that no work remains.</li>
                        <li>Both APIs return immediately when the system is uninitialized or there is no work.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Operational Meaning</h3>
                    <ul>
                        <li>Sending a message is not the same thing as processing it.</li>
                        <li>One call gives you one observable scheduling step.</li>
                        <li>The drain call is just repeated one-step processing.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Processing Modes</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>One-Step Mode</strong>
                        <span>Use the system module's <em>process next message</em> operation.</span>
                        <span>Good for tests, tracing, and exact scheduling inspection</span>
                    </div>
                    <div class="state-transition">loop boundary</div>
                    <div class="state-card">
                        <strong>Drain Mode</strong>
                        <span>Use the system module's <em>process all messages</em> operation.</span>
                        <span>Good when you want the runtime to keep consuming work until idle</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Queue Ownership Path",
        subtitle: "Message processing starts with ownership transfer into an agent queue and ends with ownership transfer back to the system.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Enqueue Path</h3>
                    <ul>
                        <li>The send-to-agent path of the agency module finds the target agent in the registry.</li>
                        <li>The send path of the agent module takes ownership of the message for that agent.</li>
                        <li>The list module appends the message at the tail, so the queue stays FIFO.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Dequeue Path</h3>
                    <ul>
                        <li>The agency module's message-pop path delegates to the agent module's queue-pop path.</li>
                        <li>The queue removes the first element through the list module's head-removal path.</li>
                        <li>The queue-pop path drops agent ownership so the system can own and destroy the processed message.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Ownership Timeline</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>1. Send</strong><br>Agency routes a message to the target agent.</div>
                    <div class="timeline-step"><strong>2. Queue</strong><br>The agent takes ownership and appends the message to its queue.</div>
                    <div class="timeline-step"><strong>3. Pop</strong><br>The agency asks the agent for the first queued message.</div>
                    <div class="timeline-step"><strong>4. Execute</strong><br>The system takes ownership, runs the method, and destroys the message afterward.</div>
                </div>
            </section>
        `
    },
    {
        title: "Scheduling Loop",
        subtitle: "The one-step scheduler walks the registered agent IDs using a rotating search hint instead of always restarting at the first agent.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Starting Position</h3>
                    <ul>
                        <li>The system asks the agency for <span class="code">first_agent_id</span>.</li>
                        <li>It prefers <span class="code">next_agent_hint</span> when that agent still exists.</li>
                        <li>If the hint is zero or stale, the scan restarts at the first registered agent.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Queue Scan</h3>
                    <ul>
                        <li>The system asks the agency module for one queued message from the current agent.</li>
                        <li>If no message is found, it advances through the agency's next-agent iterator.</li>
                        <li>When the registry iterator returns zero, the scan wraps back to the first agent.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Scan Sequence</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>1. Seed</strong><br>Choose <span class="code">next_agent_hint</span> if valid, otherwise use the first agent ID.</div>
                    <div class="timeline-step"><strong>2. Probe</strong><br>Try to pop one message from that agent.</div>
                    <div class="timeline-step"><strong>3. Advance</strong><br>Move to the next registered agent ID if the queue is empty.</div>
                    <div class="timeline-step"><strong>4. Wrap</strong><br>Loop back to the first agent ID before giving up on the full pass.</div>
                </div>
            </section>
            <p class="note">This behaves like a round-robin search cursor, but the code documents it as a search hint, not as a hard fairness guarantee.</p>
        `
    },
    {
        title: "Execution Path",
        subtitle: "Once the system has a queued message, interpreter execution becomes a straight call chain from agent ID to method AST evaluation.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>System Responsibilities</h3>
                    <ul>
                        <li>Take ownership of the popped message.</li>
                        <li>Call the interpreter module's method-execution path with the target agent ID.</li>
                        <li>Destroy the processed message after execution finishes.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Interpreter Responsibilities</h3>
                    <ul>
                        <li>Find the agent in the registry.</li>
                        <li>Load its method, mutable memory, and shared context.</li>
                        <li>Create or reset the execution frame and evaluate the method AST.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>One-Message Call Chain</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane">
                        <strong>System</strong>
                        <span>owns the scheduling step</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Agency</strong>
                        <span>returns one queued message</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Interpreter</strong>
                        <span>loads agent state and frame</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Method</strong>
                        <span>evaluates the AST</span>
                    </div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">pop queued message →</span>
                        <span class="sequence-arrow">resolve method + memory →</span>
                        <span class="sequence-arrow">evaluate instructions →</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Continuation Rules",
        subtitle: "After one execution, the system updates its search hint based on whether the same agent still has more queued work.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Same-Agent Continuation</h3>
                    <ul>
                        <li>If the agency still reports queued work for that agent, <span class="code">next_agent_hint</span> remains on that agent ID.</li>
                        <li>The next call will begin scanning from the same agent instead of searching from the top again.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Cursor Advance</h3>
                    <ul>
                        <li>If the queue is drained, the system advances to the next registered agent ID.</li>
                        <li>If there is no next ID, it wraps back to the first agent ID.</li>
                        <li>The stored hint becomes the next scan starting point.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Continuation Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Queue Still Non-Empty</strong>
                        <span>The same agent keeps the cursor.</span>
                        <span>Useful when one agent has a burst of follow-on work.</span>
                    </div>
                    <div class="state-transition">hint update</div>
                    <div class="state-card">
                        <strong>Queue Drained</strong>
                        <span>The cursor advances to the next registered agent.</span>
                        <span>The next step resumes from that new starting point.</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Delegate Fallback",
        subtitle: "If the agent scan finds no queued agent messages, the system gives delegation one chance to process queued delegate work instead.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Agent Queue Path</h3>
                    <ul>
                        <li>Positive runtime IDs route through the agency and agent registry.</li>
                        <li>Queued agent messages lead to interpreter execution.</li>
                        <li>This is the normal path for method-to-method runtime work.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Delegate Path</h3>
                    <ul>
                        <li>If no agent message is available, the system module's <em>process next message</em> operation returns whatever result comes back from the delegation module's own <em>process next message</em> path.</li>
                        <li>This keeps delegate work under the same system-level processing surface.</li>
                        <li>The system facade therefore coordinates both peers: agency and delegation.</li>
                        <li><a href="${delegationWalkthroughUrl}" target="_blank" rel="noopener noreferrer">See the Delegation walkthrough</a> for the registry, queue, and handler side of this fallback path.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Routing Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Agent Message Available</strong>
                        <span>Agency pops one queued message.</span>
                        <span>Interpreter executes the receiving agent's method.</span>
                    </div>
                    <div class="state-transition">no agent work</div>
                    <div class="state-card">
                        <strong>Agent Queues Empty</strong>
                        <span>Delegation gets the next processing opportunity.</span>
                        <span>The same API still reports whether one unit of work was processed.</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Suggested Reading Order",
        subtitle: "Read the runtime from queue insertion to dispatch if you want the exact message-processing path.",
        body: `
            <ol>
                <li><span class="code">README.md</span> for the top-level message-driven model</li>
                <li><span class="code">modules/ar_agent.h</span> and <span class="code">modules/ar_agent.c</span> for queue ownership</li>
                <li><span class="code">modules/ar_agency.h</span> and <span class="code">modules/ar_agency.c</span> for registry lookup and queue access</li>
                <li><span class="code">modules/ar_system.h</span> and <span class="code">modules/ar_system.c</span> for the scheduling loop</li>
                <li><span class="code">modules/ar_interpreter.h</span> and <span class="code">modules/ar_interpreter.c</span> for execution handoff</li>
                <li><span class="code">modules/ar_delegation.h</span> and <span class="code">modules/ar_delegation.c</span> for the fallback path</li>
            </ol>
            <p class="note">Natural follow-up walkthroughs from here would be <a href="${delegationWalkthroughUrl}" target="_blank" rel="noopener noreferrer">Delegation</a> or <span class="code">shell-runtime-flow</span>.</p>
        `
    }
];

let currentSlideIndex = 0;
let isSidebarOpen = false;

const slideElement = document.getElementById("slide");
const counterElement = document.getElementById("slide-counter");
const prevButton = document.getElementById("prev-button");
const nextButton = document.getElementById("next-button");
const menuButton = document.getElementById("menu-button");
const sidebarElement = document.getElementById("slide-nav");
const sidebarCloseButton = document.getElementById("sidebar-close");
const sidebarBackdrop = document.getElementById("sidebar-backdrop");
const sidebarListElement = document.getElementById("slide-nav-list");

function createRepositoryLink(path) {
    const link = document.createElement("a");
    link.className = "source-link";
    link.href = `${repositoryBaseUrl}${path}`;
    link.target = "_blank";
    link.rel = "noopener noreferrer";
    link.textContent = path;
    return link;
}

function linkifyRepositoryPaths(root) {
    const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
    const textNodes = [];

    while (walker.nextNode()) {
        const textNode = walker.currentNode;
        if (textNode.parentElement?.closest("a")) {
            continue;
        }
        textNodes.push(textNode);
    }

    textNodes.forEach((textNode) => {
        const pattern = new RegExp(repositoryPathPattern.source, "g");
        const text = textNode.textContent;
        let lastIndex = 0;
        let match;
        let changed = false;
        const fragment = document.createDocumentFragment();

        while ((match = pattern.exec(text)) !== null) {
            changed = true;
            fragment.append(text.slice(lastIndex, match.index));
            fragment.append(createRepositoryLink(match[0]));
            lastIndex = match.index + match[0].length;
        }

        if (!changed) {
            return;
        }

        fragment.append(text.slice(lastIndex));
        textNode.parentNode.replaceChild(fragment, textNode);
    });
}

function setSidebarOpen(nextState) {
    isSidebarOpen = nextState;
    document.body.classList.toggle("sidebar-open", isSidebarOpen);
    menuButton.setAttribute("aria-expanded", String(isSidebarOpen));
    sidebarBackdrop.hidden = !isSidebarOpen;
}

function renderSidebar() {
    sidebarListElement.innerHTML = "";

    slides.forEach((slide, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.className = "nav-link";
        if (index === currentSlideIndex) {
            button.classList.add("active");
            button.setAttribute("aria-current", "page");
        }

        button.innerHTML = `
            <span class="nav-link-title">${index + 1}. ${slide.title}</span>
            <span class="nav-link-subtitle">${slide.subtitle}</span>
        `;
        button.addEventListener("click", () => goToSlide(index));
        sidebarListElement.append(button);
    });
}

function renderSlide() {
    const slide = slides[currentSlideIndex];

    slideElement.innerHTML = `
        <h2 class="slide-title">${slide.title}</h2>
        <p class="slide-subtitle">${slide.subtitle}</p>
        ${slide.body}
    `;
    linkifyRepositoryPaths(slideElement);
    renderSidebar();

    counterElement.textContent = `${currentSlideIndex + 1} / ${slides.length}`;
    prevButton.disabled = currentSlideIndex === 0;
    nextButton.disabled = currentSlideIndex === slides.length - 1;
}

function goToSlide(index) {
    if (index < 0 || index >= slides.length) {
        return;
    }

    currentSlideIndex = index;
    renderSlide();
    setSidebarOpen(false);
}

function goToPreviousSlide() {
    goToSlide(currentSlideIndex - 1);
}

function goToNextSlide() {
    goToSlide(currentSlideIndex + 1);
}

prevButton.addEventListener("click", goToPreviousSlide);
nextButton.addEventListener("click", goToNextSlide);
menuButton.addEventListener("click", () => setSidebarOpen(!isSidebarOpen));
sidebarCloseButton.addEventListener("click", () => setSidebarOpen(false));
sidebarBackdrop.addEventListener("click", () => setSidebarOpen(false));

window.addEventListener("resize", () => {
    if (window.innerWidth > 900 && isSidebarOpen) {
        setSidebarOpen(false);
    }
});

document.addEventListener("keydown", (event) => {
    if (["ArrowLeft", "k", "K"].includes(event.key)) {
        goToPreviousSlide();
    }

    if (["ArrowRight", "j", "J"].includes(event.key)) {
        goToNextSlide();
    }

    if (event.key === "Escape" && isSidebarOpen) {
        setSidebarOpen(false);
    }
});

renderSlide();
setSidebarOpen(false);
