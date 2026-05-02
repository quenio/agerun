const repositoryBaseUrl = "https://github.com/quenio/agerun/blob/main/";
const repositoryPathPattern = /\b(?:README\.md|SPEC\.md|CHANGELOG\.md|TODO\.md|CONCEPTS\.md|MMM\.md|CLAUDE\.md|AGENTS\.md|(?:modules|methods|specs|workflows|scripts|docs|kb|models|reports|plans|docker|llama-cpp)\/[A-Za-z0-9._/-]+\.[A-Za-z0-9._-]+)\b/g;
const agentLifecycleWalkthroughUrl = "https://quenio.github.io/agerun/walkthroughs/agent-lifecycle/index.html";
const messageProcessingWalkthroughUrl = "https://quenio.github.io/agerun/walkthroughs/message-processing/index.html";

const slides = [
    {
        title: "Workflow Methodology",
        subtitle: "How the bundled workflow methodology boots, loads a definition file, evaluates item context at every configured transition, and emits visible progress logs.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Walkthrough Scope</h3>
                    <ul>
                        <li>The startup path from <span class="code">bootstrap</span> to <span class="code">workflow-coordinator</span> and <span class="code">workflow-item</span></li>
                        <li>How <span class="code">workflow-definition</span> gates startup and evaluates item context before normalizing transition decisions</li>
                        <li>How <span class="code">memory.self</span> identifies agents while <span class="code">reply_to</span> routes definition replies</li>
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
workflows/default.workflow
workflows/test.workflow</div>
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
                        <div class="anchor-middle anchor-middle-workflow">
                            <div class="map-node map-node-agent">
                                <strong>Coordinator</strong>
                                <span>owns the run status and startup handoff</span>
                            </div>
                            <div class="map-node map-node-method">
                                <strong>Definition</strong>
                                <span>loads workflow files and decides configured transitions</span>
                            </div>
                            <div class="map-node map-node-agent map-node-workflow-item">
                                <strong>Item</strong>
                                <span>stores item state and asks for transition decisions</span>
                            </div>
                            <div class="map-node map-node-agency">
                                <strong>Reporter</strong>
                                <span>publishes progress, summaries, and failures</span>
                            </div>
                            <div class="anchor-flow">start → prepare definition → initialize item → transition decision → visible log</div>
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
        title: "Workflow Methods",
        subtitle: "These methods form a message-driven methodology: bootstrap seeds the run, the coordinator prepares dependencies, the definition method validates schema and decisions, the item method carries state, and the reporter publishes visible outcomes.",
        body: `
            <div class="grid two">
                <section class="card">
                    <h3>bootstrap</h3>
                    <p>Handles the raw <span class="code">__boot__</span> startup message, spawns <span class="code">workflow-coordinator</span>, sends the bundled <span class="code">start</span> message, and emits the initial intake log line.</p>
                    <div class="path-list">methods/bootstrap.md</div>
                </section>
                <section class="card">
                    <h3>workflow-coordinator</h3>
                    <p>Stores startup metadata, spawns the definition and reporter agents, sends <span class="code">prepare_definition</span>, then spawns and initializes <span class="code">workflow-item</span> when the definition is ready.</p>
                    <div class="path-list">methods/workflow-coordinator-1.0.0.md</div>
                </section>
                <section class="card">
                    <h3>workflow-definition</h3>
                    <p>Reads workflow definition files through delegate <span class="code">-100</span>, runs <span class="code">complete(...)</span> probes, emits <span class="code">definition_ready</span> or <span class="code">definition_error</span>, and evaluates each configured transition.</p>
                    <div class="path-list">methods/workflow-definition-1.0.0.md</div>
                </section>
                <section class="card">
                    <h3>workflow-item and workflow-reporter</h3>
                    <p>The item method models the full per-item lifecycle. The reporter method converts progress, summary, and startup failure messages into visible log delegate output.</p>
                    <div class="path-list">methods/workflow-item-1.0.0.md
methods/workflow-reporter-1.0.0.md</div>
                </section>
            </div>
            <p class="note">Agents do not learn their identity from messages. The agency initializes <span class="code">memory.self</span> when each agent is created, and workflow methods use <span class="code">reply_to</span> fields only for response routing.</p>
        `
    },
    {
        title: "Bootstrap Method",
        subtitle: "bootstrap is the executable entry method: it handles the raw startup message, seeds the demo item fields, and queues the coordinator start message.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Input and Setup</h3>
                    <ul>
                        <li>Handles the raw <span class="code">__boot__</span> startup string.</li>
                        <li>Spawns <span class="code">workflow-coordinator</span> version <span class="code">1.0.0</span>.</li>
                        <li>Seeds <span class="code">definition_path=workflows/default.workflow</span>.</li>
                        <li>Seeds the bundled item values: <span class="code">demo-item-1</span>, <span class="code">demo_work_item</span>, <span class="code">high</span>, <span class="code">workflow_owner</span>, <span class="code">approved</span>.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Messages It Sends</h3>
                    <ul>
                        <li><span class="code">action=start</span> to the coordinator with definition, reporter, and item metadata.</li>
                        <li>An intake progress log message to delegate <span class="code">-102</span> so fresh boot has visible output.</li>
                        <li>Records <span class="code">demo_status</span> as either queued or failed.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Bootstrap Handoff</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane"><strong>Executable</strong><span>creates bootstrap and sends __boot__</span></div>
                    <div class="sequence-lane"><strong>bootstrap</strong><span>spawns coordinator and builds start payload</span></div>
                    <div class="sequence-lane"><strong>workflow-coordinator</strong><span>receives startup metadata</span></div>
                    <div class="sequence-lane"><strong>Log Delegate</strong><span>receives the initial intake line</span></div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">__boot__ →</span>
                        <span class="sequence-arrow">action=start →</span>
                        <span class="sequence-arrow">intake log →</span>
                    </div>
                </div>
            </section>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">methods/bootstrap.md
methods/bootstrap-1.0.0.method</div>
            </section>
        `
    },
    {
        title: "Workflow Coordinator Method",
        subtitle: "workflow-coordinator owns the run-level handoff: it prepares the definition, waits for ready/error, then launches the item lifecycle or reports startup failure.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>On action=start</h3>
                    <ul>
                        <li>Stores definition, reporter, and item metadata in memory.</li>
                        <li>Spawns <span class="code">workflow-definition</span> and <span class="code">workflow-reporter</span>.</li>
                        <li>Sends <span class="code">prepare_definition</span> to the definition agent with <span class="code">reply_to=memory.self</span>.</li>
                        <li>Sets <span class="code">run_status=waiting_for_definition</span>.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>On Definition Reply</h3>
                    <ul>
                        <li><span class="code">definition_ready</span> records workflow metadata, marks the run active, spawns <span class="code">workflow-item</span>, and sends <span class="code">initialize</span>.</li>
                        <li><span class="code">definition_error</span> marks <span class="code">startup_failed</span> and sends a startup failure event.</li>
                        <li>The coordinator never sends <span class="code">self</span> in messages; the item reads its own ID from agency-managed <span class="code">memory.self</span>.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Coordinator State Split</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Ready Path</strong>
                        <span><span class="code">run_status=active</span></span>
                        <span><span class="code">item_agent_id&gt;0</span></span>
                        <span><span class="code">initialize_sent=1</span></span>
                    </div>
                    <div class="state-transition">definition reply</div>
                    <div class="state-card">
                        <strong>Error Path</strong>
                        <span><span class="code">run_status=startup_failed</span></span>
                        <span><span class="code">startup_sent=1</span></span>
                    </div>
                </div>
            </section>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">methods/workflow-coordinator-1.0.0.md
methods/workflow-coordinator-1.0.0.method
methods/workflow_coordinator_tests.c</div>
            </section>
        `
    },
    {
        title: "Workflow Definition Method",
        subtitle: "workflow-definition is the schema gate and transition-decision method: it parses workflow files, asks complete(...) to evaluate item context, and replies to message.reply_to.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Definition Operations</h3>
                    <ul>
                        <li><span class="code">prepare_definition</span> sends a file read to delegate <span class="code">-100</span>, parses the flat record, and runs the startup dependency probe.</li>
                        <li><span class="code">describe</span> returns the parsed workflow metadata, item field list, stages, transition count, and transition path.</li>
                        <li>Missing or malformed required fields are normalized to <span class="code">invalid_definition_schema</span>.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Decision Operations</h3>
                    <ul>
                        <li><span class="code">evaluate_transition</span> builds a values map from canonical definition metadata plus current item fields.</li>
                        <li><span class="code">workflow_name</span> stays definition-backed; caller transition input cannot overwrite it.</li>
                        <li><span class="code">complete(...)</span> receives the configured transition prompt plus stage, item fields, review status, and transition count before generating <span class="code">outcome</span> and <span class="code">reason</span>.</li>
                        <li>Completion failure becomes retryable <span class="code">stay</span> with <span class="code">complete_transition_failed</span>.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Definition Method Replies</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>prepare_definition</strong><br>Reply to <span class="code">message.reply_to</span> with <span class="code">definition_ready</span> or <span class="code">definition_error</span>.</div>
                    <div class="timeline-step"><strong>evaluate_transition</strong><br>Reply to <span class="code">message.reply_to</span> with <span class="code">transition_decision</span> carrying the canonical workflow name, next stage, status, reason, retryability, and terminal outcome.</div>
                    <div class="timeline-step"><strong>describe</strong><br>Reply to <span class="code">message.reply_to</span> with definition metadata for inspection and tests.</div>
                </div>
            </section>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">methods/workflow-definition-1.0.0.md
methods/workflow-definition-1.0.0.method
methods/workflow_definition_tests.c</div>
            </section>
        `
    },
    {
        title: "Workflow Item Method",
        subtitle: "workflow-item is the stateful per-item method: it records item metadata, asks for transition decisions, and applies definition-owned next stages.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Persistent Item State</h3>
                    <ul>
                        <li>Stores workflow identity, item fields, current stage, current status, transition count, terminal outcome, and last reason.</li>
                        <li>Copies agency-managed <span class="code">memory.self</span> into <span class="code">self_agent_id</span> during initialization.</li>
                        <li>Queues its own <span class="code">auto_progress</span> messages to ask for the next configured transition.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Decision Application</h3>
                    <ul>
                        <li>At every non-terminal stage, sends <span class="code">evaluate_transition</span> to <span class="code">workflow-definition</span> with <span class="code">reply_to=self_agent_id</span>.</li>
                        <li><span class="code">advance</span> applies the returned <span class="code">next_stage</span>; terminal advances produce summary events.</li>
                        <li><span class="code">stay</span> keeps the item in the current stage and emits progress instead of a summary.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Item State Machine</h3>
                <div class="lifecycle-flow" aria-label="Workflow item stages">
                    <div class="flow-step"><strong>intake</strong><span>created</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>triage</strong><span>entered_triage</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>active</strong><span>entered_active</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>review</strong><span>ask definition</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>completion / rejected / stay</strong><span>apply decision</span></div>
                </div>
            </section>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">methods/workflow-item-1.0.0.md
methods/workflow-item-1.0.0.method
methods/workflow_item_tests.c</div>
            </section>
        `
    },
    {
        title: "Workflow Reporter Method",
        subtitle: "workflow-reporter is the visibility boundary: it converts workflow progress, summary, and startup failure events into log delegate messages.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Accepted Events</h3>
                    <ul>
                        <li><span class="code">progress</span> records and logs in-flight item state.</li>
                        <li><span class="code">summary</span> records and logs terminal item state, using fallback summary text if needed.</li>
                        <li><span class="code">startup_failure</span> records and logs dependency/schema failures without fake item fields.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>What Tests Can Inspect</h3>
                    <ul>
                        <li><span class="code">last_event_type</span>, <span class="code">last_item_id</span>, and <span class="code">last_reason</span>.</li>
                        <li><span class="code">last_message</span>, including any propagated <span class="code">COMPLETE_TRACE[...]</span>.</li>
                        <li><span class="code">delivery_status</span> after sending to the log delegate.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Reporter Boundary</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane"><strong>Coordinator / Item</strong><span>sends workflow event</span></div>
                    <div class="sequence-lane"><strong>Reporter</strong><span>formats visible text and records last event</span></div>
                    <div class="sequence-lane"><strong>Delegate -102</strong><span>receives log level, agent id, and message</span></div>
                    <div class="sequence-lane"><strong>Log Output</strong><span>shows progress, summary, or startup failure</span></div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">progress / summary / failure →</span>
                        <span class="sequence-arrow">send(-102, log_message) →</span>
                    </div>
                </div>
            </section>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">methods/workflow-reporter-1.0.0.md
methods/workflow-reporter-1.0.0.method
methods/workflow_reporter_tests.c</div>
            </section>
        `
    },
    {
        title: "Startup Sequence",
        subtitle: "Fresh executable startup is a bounded message chain: bootstrap queues the coordinator, the coordinator prepares the definition, the item runs the lifecycle, and the reporter emits the result.",
        body: `
            <section class="diagram-panel">
                <h3>Boot-to-Log Timeline</h3>
                <div class="timeline">
                    <div class="timeline-step"><strong>1. Bootstrap handles startup</strong><br>The executable creates the bootstrap agent and sends the raw <span class="code">__boot__</span> message.</div>
                    <div class="timeline-step"><strong>2. Coordinator receives start</strong><br>Bootstrap sends <span class="code">action=start</span> with definition, reporter, item, owner, priority, and review metadata.</div>
                    <div class="timeline-step"><strong>3. Definition is prepared</strong><br>The coordinator spawns <span class="code">workflow-definition</span> and sends <span class="code">prepare_definition</span> with <span class="code">workflows/default.workflow</span>.</div>
                    <div class="timeline-step"><strong>4. Ready or error returns</strong><br>The definition method replies to <span class="code">reply_to</span> with <span class="code">definition_ready</span> for known definitions and successful probes, or <span class="code">definition_error</span> on schema/probe failure.</div>
                    <div class="timeline-step"><strong>5. Item lifecycle runs</strong><br>On readiness, the coordinator spawns <span class="code">workflow-item</span> and initializes it without putting <span class="code">self</span> on the message.</div>
                    <div class="timeline-step"><strong>6. Reporter logs the visible result</strong><br>The item sends progress and summary events to the reporter; startup errors still route from the coordinator to the reporter.</div>
                </div>
            </section>
            <div class="columns">
                <section class="panel">
                    <h3>Success Output Shape</h3>
                    <p>Approved demo input becomes a completion summary such as <span class="code">workflow=default_workflow item=demo-item-1 stage=completion terminal=completed reason=&lt;complete-derived&gt;</span>.</p>
                </section>
                <section class="panel">
                    <h3>Failure Output Shape</h3>
                    <p>Startup failure becomes <span class="code">workflow_startup_failure reason=... failure_category=...</span> and does not invent fake work-item fields.</p>
                </section>
            </div>
        `
    },
    {
        title: "Definition Schema and Gate",
        subtitle: "Before the next slides compare concrete definitions, read each workflow file as one compact schema record plus explicit transition prompts.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Schema Fields</h3>
                    <ul>
                        <li><span class="code">workflow_name</span> and <span class="code">workflow_version</span> identify the definition.</li>
                        <li><span class="code">initial_stage</span>, <span class="code">stages</span>, <span class="code">terminal_completed</span>, and <span class="code">terminal_rejected</span> describe the lifecycle.</li>
                        <li><span class="code">item_fields</span> lists the item data required by the workflow methods.</li>
                        <li><span class="code">transition_count</span> and <span class="code">transition_N_from/to/prompt</span> records define the stage graph and prompt text.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Gate Checks</h3>
                    <ul>
                        <li>The definition method reads the file asynchronously through delegate <span class="code">-100</span>.</li>
                        <li>Missing metadata or transition fields become <span class="code">invalid_definition_schema</span>.</li>
                        <li><span class="code">complete("Workflow dependency probe ...")</span> supplies the startup readiness signal.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Schema-to-Reply Flow</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Parsed schema + probe success</strong>
                        <span>definition metadata and transitions are populated from the workflow file.</span>
                        <span>reply action is <span class="code">definition_ready</span>.</span>
                    </div>
                    <div class="state-transition">prepare_definition</div>
                    <div class="state-card">
                        <strong>Malformed schema or probe failure</strong>
                        <span>failure reason is normalized.</span>
                        <span>reply action is <span class="code">definition_error</span>.</span>
                    </div>
                </div>
            </section>
            <p class="note">The next two slides reuse this schema vocabulary: most fields stay the same, while the workflow name and transition prompt text distinguish the default and test definitions.</p>
        `
    },
    {
        title: "Default Workflow Definition",
        subtitle: "The default definition is the bundled executable demo path: default_workflow, intake-first staging, and four complete-backed transition prompts.",
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
                        <span><span class="code">bootstrap</span> sends <span class="code">workflows/default.workflow</span>.</span>
                        <span><span class="code">workflow-definition</span> maps it to <span class="code">default_workflow</span>.</span>
                    </div>
                    <div class="state-transition">transition table</div>
                    <div class="state-card">
                        <strong>Definition-Driven Path</strong>
                        <span><span class="code">intake→triage</span></span>
                        <span><span class="code">triage→active</span></span>
                        <span><span class="code">active→review</span></span>
                        <span><span class="code">review→completion</span></span>
                    </div>
                </div>
            </section>
            <p class="note">In the fresh executable path, the coordinator uses this default definition to stage readiness before spawning and initializing the workflow item.</p>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">workflows/default.workflow
methods/workflow-definition-1.0.0.md</div>
            </section>
        `
    },
    {
        title: "Test Workflow Definition",
        subtitle: "The test definition mirrors the default lifecycle but changes workflow identity and prompt text for deterministic alternate-definition coverage.",
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
                        <li><span class="code">transition_count=4</span></li>
                        <li>Alternate <span class="code">transition_N_prompt</span> values for prompt-selection coverage</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Definition Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Default Definition</strong>
                        <span><span class="code">default_workflow</span></span>
                        <span><span class="code">default_workflow</span> prompts</span>
                        <span><span class="code">intake→triage→active→review→completion</span></span>
                    </div>
                    <div class="state-transition">alternate fixture</div>
                    <div class="state-card">
                        <strong>Test Definition</strong>
                        <span><span class="code">test_workflow</span></span>
                        <span><span class="code">test_workflow</span> prompts</span>
                        <span><span class="code">same transition topology</span></span>
                    </div>
                </div>
            </section>
            <p class="note">This keeps test coverage focused: tests can prove alternate definition loading without changing lifecycle stages or required item fields.</p>
            <section class="card source-panel">
                <h3>Source Files</h3>
                <div class="path-list">workflows/test.workflow
methods/workflow_definition_tests.c</div>
            </section>
        `
    },
    {
        title: "Item Field Values",
        subtitle: "The schema names required item fields; the bundled demo values come from bootstrap and are carried through coordinator, item, definition, and reporter messages.",
        body: `
            <div class="columns">
                <section class="panel">
                    <h3>Bundled Demo Values</h3>
                    <ul>
                        <li><span class="code">item_id=demo-item-1</span></li>
                        <li><span class="code">title=demo_work_item</span></li>
                        <li><span class="code">priority=high</span></li>
                        <li><span class="code">owner=workflow_owner</span></li>
                        <li><span class="code">review_status=approved</span></li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>How They Are Used</h3>
                    <ul>
                        <li><span class="code">item_id</span> identifies progress and summary log messages.</li>
                        <li><span class="code">title</span>, <span class="code">priority</span>, <span class="code">owner</span>, and <span class="code">review_status</span> are stored by the item method and forwarded for transition evaluation.</li>
                        <li><span class="code">review_status=approved</span> is provided to <span class="code">complete(...)</span>, which evaluates the item context before the definition method normalizes the transition.</li>
                    </ul>
                </section>
            </div>
            <section class="diagram-panel">
                <h3>Item Field Flow</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane"><strong>bootstrap</strong><span>seeds the demo item values in memory and builds the start message</span></div>
                    <div class="sequence-lane"><strong>coordinator</strong><span>stores the fields and passes them in the item initialize message</span></div>
                    <div class="sequence-lane"><strong>workflow-item</strong><span>stores the fields, emits progress, and forwards them to transition evaluation</span></div>
                    <div class="sequence-lane"><strong>definition / reporter</strong><span>definition receives all five fields; reporter logs item_id, owner, stage, status, terminal outcome, and reason</span></div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">start message →</span>
                        <span class="sequence-arrow">initialize / evaluate_transition →</span>
                        <span class="sequence-arrow">progress or summary →</span>
                    </div>
                </div>
            </section>
            <div class="path-list">methods/bootstrap.md
methods/workflow-coordinator-1.0.0.md
methods/workflow-item-1.0.0.md
methods/workflow-definition-1.0.0.md
methods/workflow-reporter-1.0.0.md</div>
        `
    },
    {
        title: "Definition-Driven Flow",
        subtitle: "The full per-item path asks workflow-definition at each stage, and the definition file decides the only valid next stage for advance.",
        body: `
            <section class="diagram-panel">
                <h3>Per-Item Lifecycle Path</h3>
                <div class="lifecycle-flow" aria-label="Workflow item lifecycle">
                    <div class="flow-step"><strong>Initialize</strong><span>store item metadata and emit created progress</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Intake</strong><span>ask for intake→triage</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Triage</strong><span>ask for triage→active</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Active</strong><span>ask for active→review</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Review</strong><span>ask for review→completion</span></div>
                    <div class="flow-arrow">→</div>
                    <div class="flow-step"><strong>Decision</strong><span>summary on advance/reject, progress on stay</span></div>
                </div>
            </section>
            <div class="columns">
                <section class="panel">
                    <h3>Decision Outcomes</h3>
                    <ul>
                        <li><span class="code">advance</span> moves only to that stage's configured <span class="code">to</span> stage.</li>
                        <li><span class="code">reject</span> keeps the current stage and emits a rejected summary.</li>
                        <li><span class="code">stay</span> keeps the item in the current stage when context says more work is needed.</li>
                    </ul>
                </section>
                <section class="panel">
                    <h3>Identity Boundary</h3>
                    <p>The definition method keeps <span class="code">workflow_name</span> from the prepared definition. The transition message supplies item context, not authority to rename the workflow.</p>
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
        subtitle: "The workflow methodology favors explicit normalized states over silent fallthroughs or fake success records.",
        body: `
            <section class="diagram-panel">
                <h3>Outcome Comparison</h3>
                <div class="state-compare">
                    <div class="state-card">
                        <strong>Startup success</strong>
                        <span><span class="code">definition_ready</span> activates the run.</span>
                        <span>The coordinator initializes an item, and the item emits the final summary after the transition decision.</span>
                    </div>
                    <div class="state-transition">versus</div>
                    <div class="state-card">
                        <strong>Startup failure</strong>
                        <span><span class="code">definition_error</span> marks <span class="code">startup_failed</span>.</span>
                        <span>No item is created; the reporter logs a startup failure.</span>
                    </div>
                </div>
            </section>
            <div class="grid three">
                <section class="card">
                    <h3>Invalid Definition</h3>
                    <p>Missing files or malformed schema records are normalized to <span class="code">invalid_definition_schema</span>.</p>
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
        title: "Execution Story",
        subtitle: "A fresh no-persistence run turns the default bootstrap message into one visible item lifecycle in the log file.",
        body: `
            <section class="diagram-panel">
                <h3>Run Storyboard</h3>
                <div class="sequence-diagram">
                    <div class="sequence-lane"><strong>Direct run</strong><span><span class="code">./agerun --no-persistence</span> loads methods and creates <span class="code">bootstrap</span>.</span></div>
                    <div class="sequence-lane"><strong>Bootstrap</strong><span>queues <span class="code">workflow-coordinator</span> and writes the first intake line.</span></div>
                    <div class="sequence-lane"><strong>Workflow methods</strong><span>prepare the definition, initialize the item, and ask for each transition decision.</span></div>
                    <div class="sequence-lane"><strong>Reporter</strong><span>forwards progress and summary text to the log delegate.</span></div>
                    <div class="sequence-arrow-row">
                        <span class="sequence-arrow">fresh boot →</span>
                        <span class="sequence-arrow">start message →</span>
                        <span class="sequence-arrow">item lifecycle →</span>
                        <span class="sequence-arrow">visible logs →</span>
                    </div>
                </div>
            </section>
            <div class="flow-grid">
                <section class="flow-step">
                    <strong>1. Two intake markers</strong>
                    <span><span class="code">bootstrap intake → item intake</span></span>
                    <span>The first line proves the demo was queued; the second is the initialized item reporting its own state.</span>
                </section>
                <section class="flow-step">
                    <strong>2. Three advances</strong>
                    <span><span class="code">triage → active → review</span></span>
                    <span>Each line follows a separate <span class="code">evaluate_transition</span> request and configured prompt.</span>
                </section>
                <section class="flow-step">
                    <strong>3. Final transition</strong>
                    <span><span class="code">review → completion</span></span>
                    <span>The definition method normalizes the last completion result to <span class="code">outcome=advance</span>.</span>
                </section>
                <section class="flow-step">
                    <strong>4. Terminal summary</strong>
                    <span><span class="code">completion / completed</span></span>
                    <span>The reporter writes the final line with <span class="code">COMPLETE_TRACE[phase=transition|...]</span>.</span>
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
workflows/default.workflow
workflows/test.workflow</div>
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
