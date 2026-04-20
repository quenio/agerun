const repositoryBaseUrl = "https://github.com/quenio/agerun/blob/main/";
const repositoryPathPattern = /\b(?:README\.md|SPEC\.md|CHANGELOG\.md|TODO\.md|CONCEPTS\.md|MMM\.md|CLAUDE\.md|AGENTS\.md|(?:modules|methods|specs|workflows|scripts|docs|kb|models|reports|plans|docker|llama-cpp)\/[A-Za-z0-9._/-]+\.[A-Za-z0-9._-]+)\b/g;
const messageProcessingWalkthroughUrl = "https://quenio.github.io/agerun/walkthroughs/message-processing/index.html";

const slides = [
    {
        title: "Delegation",
        subtitle: "How the delegation subsystem registers delegates, queues external work, and dispatches one delegate message at a time.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Walkthrough Scope</h3>
                    <ul>
                        <li>How delegation sits beside agency inside the system runtime</li>
                        <li>How negative IDs route work to delegates instead of agents</li>
                        <li>How delegate queues preserve ownership and sender metadata</li>
                        <li>How the delegation module's <em>process next message</em> operation dispatches queued work</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Primary Source Files</h3>
                    <div class="path-list">modules/ar_delegation.h
modules/ar_delegation.c
modules/ar_delegate.h
modules/ar_delegate.c
modules/ar_delegate_registry.h
modules/ar_delegate_registry.c
modules/ar_system.h
modules/ar_system.c
modules/ar_log_delegate.h
modules/ar_log_delegate.c</div>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Delegation Surface</h3>
                <div class="reference-architecture">
                    <div class="legend-row">
                        <span class="legend-chip legend-system">System</span>
                        <span class="legend-chip legend-agency">Delegation</span>
                        <span class="legend-chip legend-agent">Delegate Registry</span>
                        <span class="legend-chip legend-method">Delegate Handler</span>
                        <span class="legend-chip legend-flow">Message Flow</span>
                    </div>
                    <div class="anchor-stack">
                        <div class="anchor-top">
                            <div class="map-node map-node-system">
                                <strong>System</strong>
                                <span>owns delegation beside agency</span>
                            </div>
                        </div>
                        <div class="anchor-middle">
                            <div class="map-node map-node-agency">
                                <strong>Delegation</strong>
                                <span>finds delegates and drains one queued item</span>
                            </div>
                            <div class="anchor-flow">register delegates → queue external work → dispatch one handler call</div>
                            <div class="map-node map-node-agent">
                                <strong>Delegate Registry</strong>
                                <span>owns delegate instances by negative ID</span>
                            </div>
                            <div class="map-node map-node-method">
                                <strong>Delegate Handler</strong>
                                <span>consumes one message with sender context</span>
                            </div>
                        </div>
                        <div class="system-map-footer">
                            <span class="map-pill">Delegation is the system's external-work peer to agency.</span>
                            <span class="map-pill"><a href="${messageProcessingWalkthroughUrl}" target="_blank" rel="noopener noreferrer">See Message Processing</a> for where delegation is used as fallback.</span>
                        </div>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Peer Runtime Model",
        subtitle: "Agency and delegation are sibling subsystems. The system owns both and chooses between them based on target type and queue availability.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Agency Side</h3>
                    <ul>
                        <li>Positive IDs identify agents.</li>
                        <li>Agency tracks registered agents and their message queues.</li>
                        <li>Agent work eventually goes through interpreter execution.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Delegation Side</h3>
                    <ul>
                        <li>Negative IDs identify delegates by convention.</li>
                        <li>Delegation tracks registered delegates and their queues.</li>
                        <li>Delegate work ends in a delegate handler, not in method AST evaluation.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>System Peer Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Agency Path</strong>
                        <span>Registry of agents with positive IDs.</span>
                        <span>Queued work becomes interpreter execution.</span>
                    </div>
                    <div class="state-transition">system facade</div>
                    <div class="state-card">
                        <strong>Delegation Path</strong>
                        <span>Registry of delegates with negative IDs.</span>
                        <span>Queued work becomes a handler callback.</span>
                    </div>
                </div>
            </section>
            <p class="note">The important category boundary is not “internal vs external code.” It is “agent method execution vs delegate handler execution.”</p>
        `
    },
    {
        title: "Registration Path",
        subtitle: "Delegates become reachable only after the system or a caller registers them into the owned delegate registry.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Creation Responsibilities</h3>
                    <ul>
                        <li>The delegation module creates and owns one delegate registry.</li>
                        <li>The system creates delegation during startup, before interpreter creation.</li>
                        <li>The system also registers built-in delegates, such as the log delegate.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Registration Semantics</h3>
                    <ul>
                        <li>The registry takes ownership of a delegate on successful registration.</li>
                        <li>Duplicate delegate IDs are rejected.</li>
                        <li>Registry destruction tears down all registered delegates.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Registration Timeline</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>1. Create Delegation</strong><br>The delegation module creates its owned registry.</div>
                    <div class="timeline-step"><strong>2. Create Delegate</strong><br>A caller builds a delegate instance, optionally with a handler and owned context.</div>
                    <div class="timeline-step"><strong>3. Register by Negative ID</strong><br>The registry stores the delegate and claims ownership on success.</div>
                    <div class="timeline-step"><strong>4. Become Addressable</strong><br>Messages can now be routed to that delegate ID.</div>
                </div>
            </section>
        `
    },
    {
        title: "Queue Ownership",
        subtitle: "Delegation routing is queue-first. Sending work to a delegate transfers message ownership into that delegate's FIFO queue.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Send Path</h3>
                    <ul>
                        <li>The delegation module looks up the target delegate in the registry.</li>
                        <li>The delegate takes ownership of the message before enqueueing it.</li>
                        <li>A second queue stores sender metadata beside the message queue.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Take Path</h3>
                    <ul>
                        <li>The delegate removes the first queued message and the matching sender metadata.</li>
                        <li>The delegate drops message ownership when returning the popped item.</li>
                        <li>The caller that pops the message must destroy it after handling.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Delegate Queue Timeline</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>1. Queue</strong><br>The delegate stores the message at the tail, preserving FIFO order.</div>
                    <div class="timeline-step"><strong>2. Record Sender</strong><br>The delegate stores either an explicit sender ID or a sentinel meaning “no explicit sender.”</div>
                    <div class="timeline-step"><strong>3. Pop</strong><br>The delegate returns the oldest message and its sender metadata together.</div>
                    <div class="timeline-step"><strong>4. Destroy After Handling</strong><br>The dispatcher destroys the returned message after the handler call completes.</div>
                </div>
            </section>
        `
    },
    {
        title: "Dispatch Loop",
        subtitle: "The delegation module's <em>process next message</em> operation scans registered delegate IDs, pops one queued item, and invokes that delegate's handler.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Scan Rules</h3>
                    <ul>
                        <li>The scan starts from the first registered delegate ID.</li>
                        <li>It checks each registered delegate in registry iteration order.</li>
                        <li>If the iterator reaches zero, the scan wraps once to the first delegate before concluding that no queued work exists.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Dispatch Rules</h3>
                    <ul>
                        <li>Only one queued delegate message is handled per call.</li>
                        <li>The handler receives the message and either the explicit sender ID or the fallback sender passed into delegation.</li>
                        <li>The operation returns true when one queued delegate message was consumed, even if the handler reports “not handled.”</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>One-Message Delegate Dispatch</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane">
                        <strong>Delegation</strong>
                        <span>scans registered delegate IDs</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Delegate Registry</strong>
                        <span>returns delegate references by ID</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Delegate Queue</strong>
                        <span>pops one message + sender metadata</span>
                    </div>
                    <div class="sequence-lane">
                        <strong>Handler</strong>
                        <span>consumes one delegate message</span>
                    </div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">probe next registered delegate →</span>
                        <span class="sequence-arrow">take first queued message →</span>
                        <span class="sequence-arrow">invoke handler →</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Sender Metadata",
        subtitle: "Delegation preserves who sent the work when that information is available, but it also supports a default sender path when no explicit sender is queued.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Explicit Sender Path</h3>
                    <ul>
                        <li>The send-with-sender path queues a concrete sender ID beside the message.</li>
                        <li>When the message is popped, that explicit sender wins.</li>
                        <li>The handler therefore sees the original sender, not a generic fallback.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Fallback Sender Path</h3>
                    <ul>
                        <li>The basic send path queues a sentinel marking the sender as unspecified.</li>
                        <li>The dispatcher then forwards the sender ID supplied by its caller.</li>
                        <li>This keeps the handler contract stable even when the queue entry itself has no explicit sender metadata.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Sender Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Explicit Sender Stored</strong>
                        <span>The queued sender ID travels with the message.</span>
                        <span>The handler receives that original sender ID.</span>
                    </div>
                    <div class="state-transition">otherwise</div>
                    <div class="state-card">
                        <strong>No Explicit Sender Stored</strong>
                        <span>The queue stores only the sentinel value.</span>
                        <span>The handler receives the dispatcher's fallback sender ID.</span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "System Fallback Integration",
        subtitle: "At the system level, delegation is the fallback path used when the agent scan finds no queued agent message to execute.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>What Happens First</h3>
                    <ul>
                        <li>The system tries to find one queued agent message, using its rotating agent hint.</li>
                        <li>If an agent message exists, interpreter execution wins and delegation is not consulted for that step.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>What Happens on Idle Agent Queues</h3>
                    <ul>
                        <li>If the agent scan finds no work, the system immediately calls delegation's <em>process next message</em> operation.</li>
                        <li>That means delegate work shares the same outer processing API as agent work.</li>
                        <li><a href="${messageProcessingWalkthroughUrl}" target="_blank" rel="noopener noreferrer">The Message Processing walkthrough</a> shows this handoff from the system perspective.</li>
                    </ul>
                </section>
            </div>
            <p class="note">So delegation is not a separate scheduler entry point for callers. It is a subsystem under the same system-level processing surface.</p>
        `
    },
    {
        title: "Suggested Reading Order",
        subtitle: "Read from the generic delegate abstraction outward if you want the full delegation mental model.",
        body: `
            <ol>
                <li><span class="code">modules/ar_delegate.h</span> and <span class="code">modules/ar_delegate.c</span> for queue ownership, handler hooks, and sender metadata</li>
                <li><span class="code">modules/ar_delegate_registry.h</span> and <span class="code">modules/ar_delegate_registry.c</span> for registry ownership and iteration order</li>
                <li><span class="code">modules/ar_delegation.h</span> and <span class="code">modules/ar_delegation.c</span> for routing and one-message dispatch</li>
                <li><span class="code">modules/ar_log_delegate.h</span> and <span class="code">modules/ar_log_delegate.c</span> for a concrete built-in delegate</li>
                <li><span class="code">modules/ar_system.h</span> and <span class="code">modules/ar_system.c</span> for how delegation plugs into system message processing</li>
            </ol>
            <p class="note">Natural companion walkthrough: <a href="${messageProcessingWalkthroughUrl}" target="_blank" rel="noopener noreferrer">Message Processing</a>.</p>
        `
    }
];
window.AgeRunWalkthroughDeck.init({
    repositoryBaseUrl,
    repositoryPathPattern,
    slides
});
