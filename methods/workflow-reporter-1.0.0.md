# workflow-reporter-1.0.0

## Overview

`workflow-reporter` converts workflow progress, summary, and startup failure events into visible log
messages sent through the log delegate (`-102`). It also records the last delivered event details in
memory so tests can assert the externally visible behavior.

## ATN Specification

The ATN below specifies the observable reporting rules for progress, summary, and startup failure
messages.

```haskell
progress_received: Boolean
summary_received: Boolean
startup_failure_received: Boolean

summary_text_empty: Boolean
fallback_summary_used: Boolean
log_message_sent: Boolean

last_event_type: String
visible_message: String
log_level: String
delivery_status: String

PROGRESS_EVENTS_ARE_CLASSIFIED_AS_PROGRESS:
  progress_received => last_event_type = "progress"

PROGRESS_EVENTS_USE_INFO_LEVEL:
  progress_received => log_level = "info"

SUMMARY_EVENTS_ARE_CLASSIFIED_AS_SUMMARY:
  summary_received => last_event_type = "summary"

EMPTY_SUMMARY_TEXT_USES_A_FALLBACK:
  summary_received and summary_text_empty => fallback_summary_used

STARTUP_FAILURE_EVENTS_ARE_CLASSIFIED_AS_FAILURES:
  startup_failure_received => last_event_type = "startup_failure"

STARTUP_FAILURES_USE_ERROR_LEVEL:
  startup_failure_received => log_level = "error"

ALL_EVENTS_ARE_FORWARDED_TO_THE_LOG_DELEGATE:
  progress_received or summary_received or startup_failure_received => log_message_sent

SUCCESSFUL_FORWARDING_UPDATES_DELIVERY_STATUS:
  log_message_sent => delivery_status = "success" or delivery_status = "error"
```

## Inputs

### `action=progress`

Expected fields:
- `workflow_name`
- `item_id`
- `stage`
- `status`
- `owner`
- `transition_count`
- `terminal_outcome`
- `reason`
- `text`

Behavior:
- stores the last progress event metadata
- forwards `message.text` to the log delegate

### `action=summary`

Expected fields:
- `workflow_name`
- `item_id`
- `stage`
- `status`
- `owner`
- `transition_count`
- `terminal_outcome`
- `reason`
- `text`

Behavior:
- stores the last summary metadata
- if `text` is empty, builds a fallback summary string
- forwards the visible summary text to the log delegate

### `action=startup_failure`

Expected fields:
- `reason`
- `failure_category`

Behavior:
- builds `workflow_startup_failure reason={reason} failure_category={failure_category}`
- logs it at `error` level
- does not invent work-item summary fields

## Stored State

The reporter keeps these values in memory for validation and diagnostics:
- `last_event_type`
- `last_item_id`
- `last_reason`
- `last_message`
- `delivery_status`

## Method Code

```agerun
memory.is_progress := if(message.action = "progress", 1, 0)
memory.is_summary := if(message.action = "summary", 1, 0)
memory.is_startup_failure := if(message.action = "startup_failure", 1, 0)
memory.last_event_type := ""
memory.last_item_id := ""
memory.last_reason := ""
memory.progress_text := ""
memory.summary_text := ""
memory.visible_message := ""
memory.last_message := ""
memory.log_level := "info"
memory.delivery_status := ""
memory.last_event_type := if(memory.is_progress = 1, "progress", memory.last_event_type)
memory.last_event_type := if(memory.is_summary = 1, "summary", memory.last_event_type)
memory.last_event_type := if(memory.is_startup_failure = 1, "startup_failure", memory.last_event_type)
memory.last_item_id := if(memory.is_progress = 1, message.item_id, memory.last_item_id)
memory.last_item_id := if(memory.is_summary = 1, message.item_id, memory.last_item_id)
memory.last_reason := if(memory.is_progress = 1, message.reason, memory.last_reason)
memory.last_reason := if(memory.is_summary = 1, message.reason, memory.last_reason)
memory.last_reason := if(memory.is_startup_failure = 1, message.reason, memory.last_reason)
memory.progress_text := if(memory.is_progress = 1, message.text, memory.progress_text)
memory.fallback_summary_text := build("workflow={workflow_name} item={item_id} stage={stage} terminal={terminal_outcome} reason={reason}", message)
memory.summary_text := if(memory.is_summary = 1, message.text, memory.summary_text)
memory.is_summary_text_empty := if(memory.summary_text = "", 1, 0)
memory.summary_text := if(memory.is_summary_text_empty = 1, memory.fallback_summary_text, memory.summary_text)
memory.startup_text := build("workflow_startup_failure reason={reason} failure_category={failure_category}", message)
memory.visible_message := if(memory.is_progress = 1, memory.progress_text, memory.visible_message)
memory.visible_message := if(memory.is_summary = 1, memory.summary_text, memory.visible_message)
memory.visible_message := if(memory.is_startup_failure = 1, memory.startup_text, memory.visible_message)
memory.last_message := memory.visible_message
memory.log_level := if(memory.is_startup_failure = 1, "error", "info")
memory.log_input := build("level={log_level} agent_id=0 message={visible_message}", memory)
memory.log_message := parse("level={level} agent_id={agent_id} message={message}", memory.log_input)
memory.log_sent := send(-102, memory.log_message)
memory.delivery_status := if(memory.log_sent = 1, "success", "error")
```

## Testing

Validated by `methods/workflow_reporter_tests.c`.

The tests verify that the reporter:
- emits progress messages
- emits final summaries
- creates fallback summaries when `text` is empty
- emits startup dependency failures with no fake work-item summary fields
- records `last_event_type`, `last_message`, and `delivery_status` for assertions
