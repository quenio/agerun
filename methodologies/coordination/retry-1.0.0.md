# Retry Method v1.0.0

## Overview

Retry executes an operation and re-executes it after failure messages until the configured policy
succeeds or reaches its limit. It supports immediate retry directly and delayed retry by composing
with scheduling.

## Behavior

Only messages with a recognized `request` value are handled as coordination requests.

On `request: "retry_start"`, the method stores operation metadata, `trace_id`, retry strategy,
maximum attempts, scheduler agent, delay ticks, and `source_agent`. It records an attempt only
after the operation send succeeds.

Matching `failure` requests retry while attempts remain. Matching `success` requests emit the
terminal response. Terminal state is recorded only after the `start` response is delivered; failed
report delivery stores a pending terminal result for retry.

## Message Format

Requests:

```text
{ request: "retry_start", operation_id: <id>, operation_target_agent: <agent>, operation_request: <request>, operation_text: <text>, max_attempts: <number>, strategy: <immediate|scheduled>, scheduler_agent: <agent>, delay_ticks: <tick>, trace_id: <trace_id>, source_agent: <agent> }
{ request: "retry_failure", trace_id: <trace_id>, attempt: <attempt>, current_tick: <tick> }
{ request: "retry_success", trace_id: <trace_id>, attempt: <attempt> }
```

Operation attempt:

```text
{ request: <operation_request>, trace_id: <trace_id>, text: <operation_text>, attempt: <number> }
```

Scheduled retry request:

```text
{
  request: "scheduling_schedule",
  schedule_id: <operation_id>,
  due_tick: <current_tick + delay_ticks>,
  target_agent: <operation_target_agent>,
  payload_request: <operation_request>,
  payload_text: <operation_text>,
  payload_attempt: <attempt>,
  trace_id: <trace_id>,
  source_agent: 0
}
```

Terminal response:

```text
{
  response: "retry_result",
  operation_id: <id>,
  trace_id: <trace_id>,
  status: <success|failure>,
  state: <succeeded|failed|dispatch_failed>,
  success_count: <0|1>,
  failure_count: <0|1>,
  attempts: <count>
}
```

## Implementation and Tests

Implementation: [`retry-1.0.0.method`](retry-1.0.0.method)

Test: [`retry_tests.c`](retry_tests.c)
