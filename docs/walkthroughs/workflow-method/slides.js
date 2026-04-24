const repositoryBaseUrl = "https://github.com/quenio/agerun/blob/main/";
const repositoryPathPattern = /\b(?:README\.md|SPEC\.md|CHANGELOG\.md|TODO\.md|CONCEPTS\.md|MMM\.md|CLAUDE\.md|AGENTS\.md|(?:modules|methods|specs|workflows|scripts|docs|kb|models|reports|plans|docker|llama-cpp)\/[A-Za-z0-9._/-]+\.[A-Za-z0-9._-]+)\b/g;
const agentLifecycleWalkthroughUrl = "https://quenio.github.io/agerun/walkthroughs/agent-lifecycle/index.html";
const messageProcessingWalkthroughUrl = "https://quenio.github.io/agerun/walkthroughs/message-processing/index.html";

const slides = [
    {
        title: "Workflow Method",
        subtitle: "How the bundled workflow method family boots, validates a definition, makes review decisions, and emits visible progress logs.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Walkthrough Scope</h3>
                    <ul>
                        <li>The startup path from <span class="code">bootstrap</span> to <span class="code">workflow-coordinator</span></li>
                        <li>How <span class="code">workflow-definition</span> gates startup and normalizes transition decisions</li>
                        <li>How <span class="code">workflow-item</span> models a per-item lifecycle in tests and direct method use</li>
                        <li>How <span class="code">workflow-reporter</span> turns workflow events into log delegate messages</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Primary Source Files</h3>
                    <div class="path-list">methods/bootstrap.md
methods/bootstrap-1.0.0.method
methods/workflow-coordinator-1.0.0.md
methods/workflow-coordinator-1.0.0.method
methods/workflow-definition-1.0.0.md
methods/workflow-definition-1.0.0.method
methods/workflow-item-1.0.0.md
methods/workflow-item-1.0.0.method
methods/workflow-reporter-1.0.0.md
methods/workflow-reporter-1.0.0.method
workflows/default-workflow.yaml
workflows/test-workflow.yaml</div>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Reference Architecture Anchor</h3>
                <div class="reference-architecture">
                    <div class="anchor-stack">
                        <div class="anchor-top">
                            <div class="map-node map-node-system">
                                <strong>Executable Boot</strong>
                                <span>creates bootstrap and queues the raw startup message</span>
                            </div>
                        </div>
                        <div class="anchor-middle">
                            <div class="map-node map-node-agent">
                                <strong>Coordinator</strong>
                                <span>owns the run status and startup handoff</span>
                            </div>
                            <div class="map-node map-node-method">
                                <strong>Definition</strong>
                                <span>loads known definitions and decides review outcomes</span>
                            </div>
                            <div class="map-node map-node-agency">
                                <strong>Reporter</strong>
                                <span>publishes progress, summaries, and failures</span>
                            </div>
                            <div class="anchor-flow">start → prepare definition → ready/error → visible log</div>
                        </div>
                        <div class="system-map-footer">
                            <span class="map-pill">Workflow methods are ordinary AgeRun methods wired together by messages.</span>
                            <span class="map-pill"><a href="${agentLifecycleWalkthroughUrl}" target="_blank" rel="noopener noreferrer">See Agent Lifecycle</a> for the underlying runtime model.</span>
                        </div>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Method Family Roles",
        subtitle: "The workflow feature is a small method family. Each method has one message-facing responsibility and keeps its state in agent memory.",
        body: `
            <div class="grid two">
                <section class="card">
                    <h3>bootstrap</h3>
                    <p>Handles the raw <span class="code">__boot__</span> startup message, spawns <span class="code">workflow-coordinator</span>, sends the bundled <span class="code">start</span> message, and emits the initial intake log line.</p>
                    <div class="path-list">methods/bootstrap.md</div>
                </section>
                <section class="card">
                    <h3>workflow-coordinator</h3>
                    <p>Stores startup metadata, spawns the definition and reporter agents, sends <span class="code">prepare_definition</span>, and records whether the run becomes active or fails at startup.</p>
                    <div class="path-list">methods/workflow-coordinator-1.0.0.md</div>
                </section>
                <section class="card">
                    <h3>workflow-definition</h3>
                    <p>Recognizes supported definition paths, runs <span class="code">complete(...)</span> probes, emits <span class="code">definition_ready</span> or <span class="code">definition_error</span>, and evaluates review-stage transitions.</p>
                    <div class="path-list">methods/workflow-definition-1.0.0.md</div>
                </section>
                <section class="card">
                    <h3>workflow-item and workflow-reporter</h3>
                    <p>The item method models the full per-item lifecycle. The reporter method converts progress, summary, and startup failure messages into visible log delegate output.</p>
                    <div class="path-list">methods/workflow-item-1.0.0.md
methods/workflow-reporter-1.0.0.md</div>
                </section>
            </div>
            <p class="note">The current fresh executable demo uses the coordinator direct-summary path after definition readiness. The separate <span class="code">workflow-item</span> method still documents and tests the fuller item lifecycle path.</p>
        `
    },
    {
        title: "Startup Sequence",
        subtitle: "Fresh executable startup is a bounded message chain: bootstrap queues the coordinator, the coordinator prepares the definition, and the reporter emits the result.",
        body: `
            <section class="diagram-panel">
                <h3>Boot-to-Log Timeline</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>1. Bootstrap handles startup</strong><br>The executable creates the bootstrap agent and sends the raw <span class="code">__boot__</span> message.</div>
                    <div class="timeline-step"><strong>2. Coordinator receives start</strong><br>Bootstrap sends <span class="code">action=start</span> with definition, reporter, item, owner, priority, and review metadata.</div>
                    <div class="timeline-step"><strong>3. Definition is prepared</strong><br>The coordinator spawns <span class="code">workflow-definition</span> and sends <span class="code">prepare_definition</span> with <span class="code">workflows/default-workflow.yaml</span>.</div>
                    <div class="timeline-step"><strong>4. Ready or error returns</strong><br>The definition method replies with <span class="code">definition_ready</span> for known definitions and successful probes, or <span class="code">definition_error</span> on schema/probe failure.</div>
                    <div class="timeline-step"><strong>5. Reporter logs the visible result</strong><br>The coordinator sends either a <span class="code">summary</span> or <span class="code">startup_failure</span> event to the reporter.</div>
                </div>
            </section>
            <div class="columns">
                <section class="panel">
                    <h3>Success Output Shape</h3>
                    <p>Approved demo input becomes a completion summary such as <span class="code">workflow=default_workflow item=demo-item-1 stage=completion terminal=completed reason=approved</span>.</p>
                </section>
                <section class="panel">
                    <h3>Failure Output Shape</h3>
                    <p>Startup failure becomes <span class="code">workflow_startup_failure reason=... failure_category=...</span> and does not invent fake work-item fields.</p>
                </section>
            </div>
        `
    },
    {
        title: "Definition Gate",
        subtitle: "workflow-definition is both the startup gate and the review-decision engine. It keeps supported definitions intentionally narrow and path based.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Known Definitions</h3>
                    <ul>
                        <li><span class="code">workflows/default-workflow.yaml</span> maps to <span class="code">default_workflow</span> and <span class="code">review_gate</span>.</li>
                        <li><span class="code">workflows/test-workflow.yaml</span> maps to <span class="code">test_workflow</span> and <span class="code">test_gate</span>.</li>
                        <li>Unknown paths and <span class="code">invalid-workflow.yaml</span> become <span class="code">invalid_definition_schema</span>.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Startup Probe</h3>
                    <ul>
                        <li><span class="code">complete("Workflow dependency probe ...")</span> supplies the readiness signal.</li>
                        <li>Success produces <span class="code">definition_ready</span>.</li>
                        <li>Failure produces <span class="code">startup_dependency_unavailable</span>.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Definition Decision Split</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Known path + probe success</strong>
                        <span>metadata is populated</span>
                        <span>reply action is <span class="code">definition_ready</span></span>
                    </div>
                    <div class="state-transition">prepare_definition</div>
                    <div class="state-card">
                        <strong>Unknown path or probe failure</strong>
                        <span>failure reason is normalized</span>
                        <span>reply action is <span class="code">definition_error</span></span>
                    </div>
                </div>
            </section>
        `
    },
    {
        title: "Default Workflow Definition",
        subtitle: "The default definition is the bundled executable demo path: default_workflow, intake-first staging, review_gate validation, and workflow_review decision text.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Definition Identity</h3>
                    <ul>
                        <li><span class="code">workflow_name=default_workflow</span></li>
                        <li><span class="code">workflow_version=1.0.0</span></li>
                        <li><span class="code">initial_stage=intake</span></li>
                        <li><span class="code">requires_local_completion=1</span></li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Item Contract</h3>
                    <ul>
                        <li>Required item fields are <span class="code">item_id</span>, <span class="code">title</span>, <span class="code">priority</span>, <span class="code">owner</span>, and <span class="code">review_status</span>.</li>
                        <li>The stage list is <span class="code">intake|triage|active|review|completion</span>.</li>
                        <li>Terminal names are <span class="code">completed</span> and <span class="code">rejected</span>.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Default Definition Shape</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Bundled Demo Path</strong>
                        <span><span class="code">bootstrap</span> sends <span class="code">workflows/default-workflow.yaml</span>.</span>
                        <span><span class="code">workflow-definition</span> maps it to <span class="code">default_workflow</span>.</span>
                    </div>
                    <div class="state-transition">review gate</div>
                    <div class="state-card">
                        <strong>Decision Template</strong>
                        <span><span class="code">validation_clause=review_gate</span></span>
                        <span><span class="code">decision_template=workflow_review_{outcome}_{reason}</span></span>
                    </div>
                </div>
            </section>
            <p class="note">In the current fresh executable path, the coordinator uses this default definition to stage readiness before emitting the direct approved/rejected summary.</p>
            <div class="path-list">workflows/default-workflow.yaml
methods/workflow-definition-1.0.0.md</div>
        `
    },
    {
        title: "Test Workflow Definition",
        subtitle: "The test definition mirrors the default lifecycle but changes the workflow identity, validation clause, and decision template for deterministic alternate-definition coverage.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>What Stays the Same</h3>
                    <ul>
                        <li><span class="code">workflow_version=1.0.0</span></li>
                        <li><span class="code">initial_stage=intake</span></li>
                        <li><span class="code">requires_local_completion=1</span></li>
                        <li><span class="code">stages=intake|triage|active|review|completion</span></li>
                        <li><span class="code">terminal_completed=completed</span> and <span class="code">terminal_rejected=rejected</span></li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>What Changes</h3>
                    <ul>
                        <li><span class="code">workflow_name=test_workflow</span></li>
                        <li><span class="code">validation_clause=test_gate</span></li>
                        <li><span class="code">decision_template=test_review_{outcome}_{reason}</span></li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Definition Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Default Definition</strong>
                        <span><span class="code">default_workflow</span></span>
                        <span><span class="code">review_gate</span></span>
                        <span><span class="code">workflow_review_{outcome}_{reason}</span></span>
                    </div>
                    <div class="state-transition">alternate fixture</div>
                    <div class="state-card">
                        <strong>Test Definition</strong>
                        <span><span class="code">test_workflow</span></span>
                        <span><span class="code">test_gate</span></span>
                        <span><span class="code">test_review_{outcome}_{reason}</span></span>
                    </div>
                </div>
            </section>
            <p class="note">This keeps test coverage focused: tests can prove alternate definition loading without changing lifecycle stages or required item fields.</p>
            <div class="path-list">workflows/test-workflow.yaml
methods/workflow_definition_tests.c</div>
        `
    },
    {
        title: "Review Decision Flow",
        subtitle: "The full per-item path advances automatically to review, then asks workflow-definition to normalize a generated transition decision.",
        body: `
            <section class="diagram-panel">
                <h3>Per-Item Lifecycle Path</h3>
                <div class="lifecycle-flow" aria-label="Workflow item lifecycle">
                    <div class="flow-step"><strong>Initialize</strong><span>store item metadata and emit created progress</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Intake</strong><span>auto-progress to triage</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Triage</strong><span>auto-progress to active</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Active</strong><span>auto-progress to review</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Review</strong><span>send evaluate_transition to definition</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Decision</strong><span>summary on advance/reject, progress on stay</span></div>
                </div>
            </section>
            <div class="columns">
                <section class="panel">
                    <h3>Decision Outcomes</h3>
                    <ul>
                        <li><span class="code">advance</span> moves review to completion and emits a completed summary.</li>
                        <li><span class="code">reject</span> keeps the current stage and emits a rejected summary.</li>
                        <li><span class="code">stay</span> keeps the item in review and emits a retryable progress update.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Trace Propagation</h3>
                    <p>Transition decisions can carry <span class="code">COMPLETE_TRACE[phase=transition|...]</span>, and the item method forwards that trace to reporter-facing progress or summary messages.</p>
                </section>
            </div>
        `
    },
    {
        title: "Reporter Output",
        subtitle: "workflow-reporter is the boundary between workflow state and visible logs. It stores the last event and forwards the message to delegate -102.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Reporter Inputs</h3>
                    <ul>
                        <li><span class="code">progress</span> events are logged at info level.</li>
                        <li><span class="code">summary</span> events are logged at info level and get fallback text when needed.</li>
                        <li><span class="code">startup_failure</span> events are logged at error level.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Stored Diagnostics</h3>
                    <ul>
                        <li><span class="code">last_event_type</span></li>
                        <li><span class="code">last_item_id</span></li>
                        <li><span class="code">last_reason</span></li>
                        <li><span class="code">last_message</span></li>
                        <li><span class="code">delivery_status</span></li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Visible Log Boundary</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane"><strong>Coordinator or Item</strong><span>sends progress, summary, or startup_failure</span></div>
                    <div class="sequence-lane"><strong>Reporter</strong><span>builds visible message and event state</span></div>
                    <div class="sequence-lane"><strong>Log Delegate -102</strong><span>receives level, agent_id, and message</span></div>
                    <div class="sequence-lane"><strong>Log File</strong><span>records user-visible workflow progress</span></div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">event message →</span>
                        <span class="sequence-arrow">send(-102, log_message) →</span>
                        <span class="sequence-arrow">write log output →</span>
                    </div>
                </div>
            </section>
            <p class="note">See the delegation walkthrough for the runtime mechanics behind negative delegate IDs and delegate message queues.</p>
        `
    },
    {
        title: "Success, Failure, and Retry",
        subtitle: "The workflow method family favors explicit normalized states over silent fallthroughs or fake success records.",
        body: `
            <section class="diagram-panel">
                <h3>Outcome Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Startup success</strong>
                        <span><span class="code">definition_ready</span> activates the run.</span>
                        <span>The coordinator emits a final summary in the current bundled executable path.</span>
                    </div>
                    <div class="state-transition">versus</div>
                    <div class="state-card">
                        <strong>Startup failure</strong>
                        <span><span class="code">definition_error</span> marks <span class="code">startup_failed</span>.</span>
                        <span>No fake item is created; the reporter logs a startup failure.</span>
                    </div>
                </div>
            </section>
            <div class="grid three">
                <section class="card">
                    <h3>Invalid Definition</h3>
                    <p>Unknown paths are normalized to <span class="code">invalid_definition_schema</span>.</p>
                </section>
                <section class="card">
                    <h3>Dependency Failure</h3>
                    <p>A failed startup <span class="code">complete(...)</span> probe becomes <span class="code">startup_dependency_unavailable</span>.</p>
                </section>
                <section class="card">
                    <h3>Transition Failure</h3>
                    <p>A failed transition <span class="code">complete(...)</span> call becomes retryable <span class="code">stay</span> with <span class="code">complete_transition_failed</span>.</p>
                </section>
            </div>
        `
    },
    {
        title: "Suggested Reading Order",
        subtitle: "Read the workflow methods in message order, then use tests to verify the branches and memory fields.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Runtime Reading Path</h3>
                    <ol>
                        <li><span class="code">methods/bootstrap.md</span> for the executable startup entry point</li>
                        <li><span class="code">methods/workflow-coordinator-1.0.0.md</span> for run orchestration</li>
                        <li><span class="code">methods/workflow-definition-1.0.0.md</span> for definitions and decisions</li>
                        <li><span class="code">methods/workflow-reporter-1.0.0.md</span> for visible log output</li>
                        <li><span class="code">methods/workflow-item-1.0.0.md</span> for the fuller per-item lifecycle</li>
                    </ol>
                </section>
                <section class="panel">
                    <h3>Verification Reading Path</h3>
                    <div class="path-list">methods/bootstrap_tests.c
methods/workflow_coordinator_tests.c
methods/workflow_definition_tests.c
methods/workflow_item_tests.c
methods/workflow_reporter_tests.c
workflows/default-workflow.yaml
workflows/test-workflow.yaml</div>
                </section>
            </div>
            <section class="read-next">
                <h2>Next Walkthroughs</h2>
                <p>Use <a href="${messageProcessingWalkthroughUrl}" target="_blank" rel="noopener noreferrer">Message Processing</a> to understand how each queued workflow message is selected and executed. Use the delegation walkthrough to understand the log delegate boundary.</p>
            </section>
        `
    }
];

window.AgeRunWalkthroughDeck.init({
    repositoryBaseUrl,
    repositoryPathPattern,
    slides
});
