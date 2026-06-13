# Retry Method v1.0.0

## Overview

Retry executes an operation and re-executes it after failure messages until the configured policy
succeeds or reaches its limit. It supports immediate retry directly and delayed retry by composing
with scheduling.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "retry_start"`, the method stores operation metadata, effective `trace_id`,
`session_id`, retry strategy, maximum attempts, scheduler agent, delay ticks, and `sender`. It
records an attempt only after the operation send succeeds.

Matching `failure` requests with the same `session_id` retry while attempts remain. Matching
`success` requests with the same `session_id` emit the terminal response. Terminal state is
recorded only after the response is delivered; failed report delivery stores a pending terminal
result for retry.

## Message Format

Requests:

```text
{ sender: <sender-agent>, request: "retry_start", trace_id: <trace_id>, session_id: <session_id>, operation_id: <id>, operation_recipient: <agent>, operation_request: <request>, operation_text: <text>, max_attempts: <number>, strategy: <immediate|scheduled>, scheduler_agent: <agent>, delay_ticks: <tick> }
{ sender: <sender-agent>, request: "retry_failure", trace_id: <trace_id>, session_id: <session_id>, attempt: <attempt>, current_tick: <tick> }
{ sender: <sender-agent>, request: "retry_success", trace_id: <trace_id>, session_id: <session_id>, attempt: <attempt> }
```

Operation attempt:

```text
{ sender: <sender-agent>, request: <operation_request>, trace_id: <trace_id>, session_id: <session_id>, text: <operation_text>, attempt: <number> }
```

Scheduled retry request:

```text
{
  sender: <sender-agent>,
  request: "scheduling_schedule",
  trace_id: <trace_id>,
  session_id: <session_id>,
  schedule_id: <operation_id>,
  due_tick: <current_tick + delay_ticks>,
  recipient: <recipient-agent>,
  payload_request: <operation_request>,
  payload_text: <operation_text>,
  payload_attempt: <attempt>
}
```

Terminal response:

```text
{
  sender: <retry-agent>,
  response: "retry_result",
  trace_id: <trace_id>,
  session_id: <session_id>,
  status: <success|failure>,
  state: <succeeded|failed|dispatch_failed>,
  operation_id: <id>,
  success_count: <0|1>,
  failure_count: <0|1>,
  attempts: <count>
}
```

## Implementation and Tests

Implementation: [`retry-1.0.0.method`](retry-1.0.0.method)

Test: [`retry_tests.c`](retry_tests.c)
