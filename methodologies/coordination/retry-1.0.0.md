# Retry Method v1.0.0

## Overview

The retry method executes an operation and re-executes it after failure messages until the configured
retry policy succeeds or reaches its limit. It supports immediate retry directly and delayed retry by
composition with scheduling.

## Behavior

On a map whose `action` field is `"start"`, the method stores the operation metadata, retry
strategy, maximum attempts, scheduler agent, delay tick, and reply target. It sets `attempts=1` and
sends the first operation attempt.

On a map whose `action` field is `"failure"`, the method retries when `attempts < max_attempts`. For
`strategy=immediate`, it sends the next attempt directly to the operation target. For
`strategy=scheduled`, it sends a schedule request to the scheduler agent. If no attempts remain, it
reports `status=failed`.

On a map whose `action` field is `"success"`, it reports `status=succeeded` with the current attempt
count.

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
{ action: "failure" }
{ action: "success" }
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
  due_tick: <delay_ticks>,
  target: <operation_target>,
  payload_action: <operation_action>,
  payload_text: <operation_text>,
  correlation_id: <operation_id>,
  reply_to: 0
}
```

Final result:

```text
{
  action: "retry_result",
  operation_id: <id>,
  status: <succeeded|failed>,
  attempts: <count>
}
```

## Composition Notes

Use retry directly around unreliable workers, workflow steps, or outbound operations. For delayed
retry, pair it with a scheduling agent and a tick source.

## Limitations

The method supports immediate retry and a scheduled retry handoff. Backoff policies depend on an
external tick convention and richer policy arithmetic than this bounded method implements.

## Implementation and Tests

Implementation: [`retry-1.0.0.method`](retry-1.0.0.method)

Test: [`retry_tests.c`](retry_tests.c)
