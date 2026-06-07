# Retry Method v1.0.0

## Overview

The retry method executes an operation and re-executes it after failure messages until the configured
retry policy succeeds or reaches its limit. It supports immediate retry directly and delayed retry by
composition with scheduling.

## Behavior

On a map whose `action` field is `"start"`, the method stores the operation metadata, retry
strategy, maximum attempts, scheduler agent, delay tick, and reply target. It sends the first
operation attempt with `attempt: 1` and records `attempts=1` only after that send succeeds. If the
initial operation send fails, it reports `status=dispatch_failed` with `attempts=0`.

On a map whose `action` field is `"failure"` and whose `correlation_id` matches the active
operation, the method retries when `attempts < max_attempts`. For `strategy=immediate`, it sends the
next attempt directly to the operation target. For
`strategy=scheduled`, it sends a schedule request to the scheduler agent with `due_tick` set to the
failure message's `current_tick` plus `delay_ticks` and `payload_attempt` set to the next attempt
number. If no attempts remain, it reports `status=failed`.
The attempt count advances only after the immediate retry or scheduled retry handoff is sent
successfully. A failed retry dispatch leaves the retry active at the previous attempt count.

On a map whose `action` field is `"success"` and whose `correlation_id` matches the active operation,
it reports `status=succeeded` with the current attempt count.

Once the retry state reaches terminal `succeeded` or `failed` status, later stale `failure` or
`success` outcome messages are ignored. A new `start` request opens a fresh active retry state. Late
outcomes from a previous operation are ignored because their `correlation_id` no longer matches the
active operation id.
Terminal status is recorded only after the `retry_result` report is delivered. If report delivery
fails, the retry stores the pending terminal status and attempt count. Later matching outcomes retry
that original terminal report without changing the attempt count or replacing the pending terminal
result.

## Message Format

Start request:

```text
{
  action: "start",
  operation_id: <id>,
  operation_target: <agent>,
  operation_action: <action>,
  operation_text: <text>,
  max_attempts: <number>,
  strategy: <immediate|scheduled>,
  scheduler_agent: <agent>,
  delay_ticks: <tick>,
  reply_to: <agent>
}
```

Outcome requests:

```text
{ action: "failure", correlation_id: <operation_id>, current_tick: <tick> }
{ action: "success", correlation_id: <operation_id> }
```

Operation attempt:

```text
{
  action: <operation_action>,
  correlation_id: <operation_id>,
  text: <operation_text>,
  attempt: <number>
}
```

Scheduled retry request:

```text
{
  action: "schedule",
  schedule_id: <operation_id>,
  due_tick: <current_tick + delay_ticks>,
  target: <operation_target>,
  payload_action: <operation_action>,
  payload_text: <operation_text>,
  payload_attempt: <attempt>,
  correlation_id: <operation_id>,
  reply_to: 0
}
```

Final result:

```text
{
  action: "retry_result",
  operation_id: <id>,
  status: <succeeded|failed|dispatch_failed>,
  attempts: <count>
}
```

## Action Field

The input `action` field is a command discriminator in the request map. The retry agent runs this
method for every message it receives, so the field separates retry setup, failure, and success
signals and prevents unrelated messages from incrementing attempts or reporting final status.

## Composition Notes

Use retry directly around unreliable workers, workflow steps, or outbound operations. For delayed
retry, pair it with a scheduling agent and a tick source.

## Limitations

The method supports immediate retry and a scheduled retry handoff. Backoff policies depend on an
external tick convention and richer policy arithmetic than this bounded method implements.

## Implementation and Tests

Implementation: [`retry-1.0.0.method`](retry-1.0.0.method)

Test: [`retry_tests.c`](retry_tests.c)
