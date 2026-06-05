# Distribution Method v1.0.0

## Overview

The distribution method assigns caller-provided work portions to worker agents. It composes with the
routing method for delivery, making fan-out reusable without duplicating route construction in every
agency.

## Behavior

On `action=distribute`, the method stores the work id, routing agent, and reply target. It then
creates one route request for each nonzero worker slot `worker_a`, `worker_b`, and `worker_c`.

Each route request has `payload_action=work`, uses the worker portion as `payload_text`, and uses the
work id as the correlation id. The method replies with the assignment count and send outcomes.

## Message Format

Distribution request:

```text
action=distribute work_id=<id> routing_agent=<agent> reply_to=<agent> worker_a=<agent> portion_a=<text> worker_b=<agent> portion_b=<text> worker_c=<agent> portion_c=<text>
```

Route message sent to routing:

```text
action=route mode=one target=<worker> payload_action=work payload_text=<portion> correlation_id=<work_id> reply_to=0
```

Result response:

```text
action=distribution_result status=distributed work_id=<id> assignment_count=<count> sent_a=<0|1> sent_b=<0|1> sent_c=<0|1>
```

## Composition Notes

Pair distribution with aggregation for fan-out and fan-in. A workflow step can send work to a
distribution agent, workers can return `action=result` messages to an aggregation agent, and
aggregation can emit completion when enough results arrive.

## Limitations

The method assigns already-partitioned portions. Dynamic decomposition, arbitrary worker lists, and
load-aware placement require collection iteration or another decomposition method.

## Implementation and Tests

Implementation: [`distribution-1.0.0.method`](distribution-1.0.0.method)

Test: [`distribution_tests.c`](distribution_tests.c)

