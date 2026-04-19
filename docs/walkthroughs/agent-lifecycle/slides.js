const repositoryBaseUrl = "https://github.com/quenio/agerun/blob/main/";
const repositoryPathPattern = /\b(?:README\.md|SPEC\.md|CHANGELOG\.md|TODO\.md|CONCEPTS\.md|MMM\.md|CLAUDE\.md|AGENTS\.md|(?:modules|methods|specs|workflows|scripts|docs|kb|models|reports|plans|docker|llama-cpp)\/[A-Za-z0-9._/-]+\.[A-Za-z0-9._-]+)\b/g;

const slides = [
    {
        title: "Agent Lifecycle",
        subtitle: "A first-pass walkthrough of how an AgeRun agent is created, receives work, runs its method, and remains part of the runtime.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Walkthrough Scope</h3>
                    <ul>
                        <li>What an agent is in AgeRun</li>
                        <li>How methods, agency, and system fit together</li>
                        <li>What happens from creation to message processing</li>
                        <li>Which files to read next for deeper detail</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Primary Source Files</h3>
                    <div class="path-list">modules/ar_agent.h
modules/ar_agent.c
modules/ar_agency.h
modules/ar_agency.c
modules/ar_system.h
modules/ar_system.c
modules/ar_method.h
modules/ar_method.c</div>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Reference Architecture Anchor</h3>
                <div class="reference-architecture">
                    <div class="legend-row">
                        <span class="legend-chip legend-system">System</span>
                        <span class="legend-chip legend-agency">Agency</span>
                        <span class="legend-chip legend-agent">Agent</span>
                        <span class="legend-chip legend-method">Method</span>
                        <span class="legend-chip legend-flow">Message flow</span>
                    </div>
                    <div class="anchor-stack">
                        <div class="anchor-top">
                            <div class="map-node map-node-system">
                                <strong>System</strong>
                                <span>selects and processes work</span>
                            </div>
                        </div>
                        <div class="anchor-middle">
                            <div class="map-node map-node-agency">
                                <strong>Agency</strong>
                                <span>tracks active agents</span>
                            </div>
                            <div class="anchor-flow">message delivery → processing → follow-on work</div>
                            <div class="map-node map-node-agent">
                                <strong>Agent</strong>
                                <span>identity + memory + queue</span>
                            </div>
                            <div class="map-node map-node-method">
                                <strong>Method</strong>
                                <span>bound behavior version</span>
                            </div>
                        </div>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Mental Model",
        subtitle: "An agent is a runtime entity. A method is the versioned behavior it executes. The system and agency coordinate delivery and execution.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Core Roles</h3>
                    <ul>
                        <li><strong>Method</strong>: versioned executable definition</li>
                        <li><strong>Agent</strong>: stateful runtime instance using one method</li>
                        <li><strong>Agency</strong>: collection manager for active agents</li>
                        <li><strong>System</strong>: message-processing runtime that drives execution</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Working Model</h3>
                    <p>Think of an agent as: <span class="code">method + memory + queued messages + identity</span>.</p>
                    <p class="note">That is not the literal struct definition; it is the right conceptual model for reading the runtime.</p>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Runtime Relationship Map</h3>
                <p class="diagram-caption">This reuses the same visual vocabulary from the anchor slide so later diagrams stay easy to parse.</p>
                <div class="system-map">
                    <div class="map-node map-node-system">
                        <strong>System</strong>
                        <span>drives processing</span>
                    </div>
                    <div class="map-arrow map-arrow-down">invokes</div>
                    <div class="system-map-row">
                        <div class="map-node map-node-agency">
                            <strong>Agency</strong>
                            <span>tracks active agents</span>
                        </div>
                        <div class="map-node map-node-agent">
                            <strong>Agent</strong>
                            <span>identity + memory + queue</span>
                        </div>
                        <div class="map-node map-node-method">
                            <strong>Method</strong>
                            <span>versioned behavior</span>
                        </div>
                    </div>
                    <div class="system-map-footer">
                        <span class="map-pill">messages flow into the agent queue</span>
                        <span class="map-pill">the agent runs its bound method</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Lifecycle Overview",
        subtitle: "Most agent behavior can be understood as a repeating loop around message delivery and processing.",
        body: `
            <div class="lifecycle-flow" aria-label="Agent lifecycle flow">
                <div class="flow-step">
                    <strong>1. Method exists</strong>
                    <span>definition + version are available</span>
                </div>
                <div class="flow-arrow">→</div>
                <div class="flow-step">
                    <strong>2. Agent created</strong>
                    <span>identity, memory, queue, method binding</span>
                </div>
                <div class="flow-arrow">→</div>
                <div class="flow-step">
                    <strong>3. Message queued</strong>
                    <span>work arrives asynchronously</span>
                </div>
                <div class="flow-arrow">→</div>
                <div class="flow-step">
                    <strong>4. System processes</strong>
                    <span>runtime picks the next message</span>
                </div>
                <div class="flow-arrow">→</div>
                <div class="flow-step">
                    <strong>5. Method runs</strong>
                    <span>state changes, messages may be sent</span>
                </div>
                <div class="flow-arrow">→</div>
                <div class="flow-step">
                    <strong>6. Agent persists or exits</strong>
                    <span>ready for more work or lifecycle removal</span>
                </div>
            </div>
            <div class="loop-note">Most agents repeat steps 3 through 5 many times before they ever leave the runtime.</div>
        `
    },
    {
        title: "Creation Path",
        subtitle: "Creation is where a method definition becomes a live runtime participant.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Creation Preconditions</h3>
                    <ul>
                        <li>The method name is known</li>
                        <li>The requested version resolves successfully</li>
                        <li>The runtime context needed by that method is available</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Creation Outcomes</h3>
                    <ul>
                        <li>a runtime agent ID</li>
                        <li>agent-owned state/memory</li>
                        <li>registration inside the agency</li>
                        <li>a target that can now receive messages</li>
                    </ul>
                </section>
            </div>
            <p class="note">In practice, agent creation is not the same as immediate execution. Execution happens when messages are processed.</p>
        `
    },
    {
        title: "Message Delivery",
        subtitle: "AgeRun is message-driven. Sending work and running work are separate steps.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Delivery-Execution Separation</h3>
                    <ul>
                        <li><strong>Send</strong> queues a message for an agent</li>
                        <li><strong>Process</strong> makes the system actually execute the next queued message</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Operational Implications</h3>
                    <ul>
                        <li>You can inspect flow as discrete runtime steps</li>
                        <li>Multiple sends may accumulate before processing</li>
                        <li>Tests must usually process messages explicitly</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>One-Message Sequence</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane">
                        <strong>Sender</strong>
                        <span>origin agent or runtime</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Agency / System</strong>
                        <span>routes and later processes</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Target Agent</strong>
                        <span>owns queue + memory</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Method</strong>
                        <span>handles the message</span>
                    </div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">send → queued</span>
                        <span class="sequence-arrow">next message selected →</span>
                        <span class="sequence-arrow">invoke bound method →</span>
                    </div>
                </div>
            </section>
            <div class="path-list">See also:
modules/ar_system.c
modules/ar_agency.c
README.md (message-driven architecture and usage examples)</div>
        `
    },
    {
        title: "Execution Step",
        subtitle: "When the system processes the next message, it hands that message to the receiving agent's method logic.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Method Run Inputs</h3>
                    <ul>
                        <li>the receiving agent</li>
                        <li>the queued message</li>
                        <li>the agent memory/state</li>
                        <li>the shared runtime context</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Execution Outcomes</h3>
                    <ul>
                        <li>memory variables updated</li>
                        <li>outbound messages sent</li>
                        <li>new agents spawned</li>
                        <li>termination or lifecycle operations requested</li>
                    </ul>
                </section>
            </div>
        `
    },
    {
        title: "Persistence and Long-Lived Agents",
        subtitle: "Agents are not just transient callbacks; they can outlive one message and participate in a persistent runtime.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Persistence Value</h3>
                    <ul>
                        <li>agent memory carries forward across messages</li>
                        <li>runtime state can be saved and restored</li>
                        <li>long-lived workflows become possible</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Further Reading</h3>
                    <div class="path-list">README.md
modules/ar_agency.md
modules/ar_system.md
methods/bootstrap-1.0.0.method</div>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Persistence Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Fresh boot</strong>
                        <span>bootstrap agent is created</span>
                        <span>startup message is queued and processed</span>
                    </div>
                    <div class="state-transition">save / restore</div>
                    <div class="state-card">
                        <strong>Restored runtime</strong>
                        <span>agents come back with prior memory</span>
                        <span>the runtime continues instead of starting from zero</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Suggested Reading Order",
        subtitle: "If you want to understand agent lifecycle deeply, follow the runtime path in this order.",
        body: `
            <ol>
                <li><span class="code">README.md</span> for the top-level runtime model</li>
                <li><span class="code">modules/ar_agent.md</span> for the agent abstraction</li>
                <li><span class="code">modules/ar_agency.md</span> for how agents are tracked and addressed</li>
                <li><span class="code">modules/ar_system.md</span> for processing flow</li>
                <li><span class="code">methods/bootstrap-1.0.0.method</span> to see a concrete startup path</li>
            </ol>
            <p class="note">From here, a natural follow-up walkthrough would be <span class="code">message-processing</span> or <span class="code">method-loading</span>.</p>
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
