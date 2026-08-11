# App Event Bus Specification

## Overview

The event_bus module provides publish/subscribe delivery based on full topic
strings.

- Topics are stored as complete paths such as `audio`, `audio/volume`, or
  `audio/*`.
- `/` expresses hierarchy inside one topic string.
- The module supports both synchronous and asynchronous subscribers.
- Topics and subscribers are both stored in singly linked lists.
- The current implementation exposes a generic async transport hook plus
  `event_bus_async_dispatch()` for the receiving task side.

This document consolidates the design decisions, edge cases, and risk analysis
that were explored during the current event_bus review and refactor.

Detailed review notes, async integration patterns, and memory/concurrency risk
analysis are maintained in [DESIGN_NOTES.md](DESIGN_NOTES.md).

## Topic Model

### Full-path topics

The current design treats every topic as one complete string.

- `audio` is a standalone topic.
- `audio/volume` is a different topic.
- `audio/state/mode` is valid and there is no fixed depth limit.

Parent topics do not automatically match child topics.

- Publishing `audio/volume` does not match `audio`.
- Publishing `audio/state/mode` does not match `audio/state`.

### Wildcard rule

The only wildcard currently implemented is a terminal `/*` suffix.

- `audio/*` matches `audio/volume`.
- `audio/*` matches `audio/state/mode`.
- `audio/*` does not match `audio`.
- `audio/*` does not match `video/volume`.

The reserved `**` style wildcard is not implemented in the current version.

## Registration Rules

Topic registration is strict.

- A topic must be registered through `event_bus_topic_register()` before any
  subscription is allowed.
- `event_bus_subscribe()` and `event_bus_subscribe_async()` return
  `EVENT_BUS_ERR_TOPIC_NOT_FOUND` when the topic was not registered first.
- Registering the same topic twice is treated as success and keeps the
  existing node.

Examples:

- If `audio/volume` is not registered, subscribing to `audio/volume` fails.
- If wildcard delivery is needed, `audio/*` must also be registered as its own
  topic node.

## Publish Semantics

`event_bus_publish()` scans every registered topic node and checks whether that
node matches the published topic.

- Exact matches are delivered.
- Registered wildcard topics with `/*` are delivered.
- Delivery succeeds when at least one registered topic matches.
- A matched topic with zero subscribers still counts as a successful publish.
- If no registered topic matches, `event_bus_publish()` returns
  `EVENT_BUS_ERR_TOPIC_NOT_FOUND`.

## Ordering Rules

Both the topic list and the subscriber list use head insertion.

### Topic order

Newly registered topics are inserted at the head of `s_topic_list`.

Example registration order:

1. `audio/volume`
2. `audio/state`
3. `audio/mode`

Internal traversal order becomes:

1. `audio/mode`
2. `audio/state`
3. `audio/volume`

So the current publish order across topic nodes is LIFO.

### Subscriber order

New subscribers are inserted at the head of the matched topic's subscriber
list.

If three callbacks are subscribed to the same topic in order `A`, `B`, `C`, the
execution order is `C`, then `B`, then `A`.

## Duplicate Callback Behavior

The module does not deduplicate callbacks.

- The same callback may be subscribed to multiple topics.
- The same callback may also be subscribed multiple times to the same topic.
- If multiple subscriptions match one publish, that callback is invoked once
  per matching subscription.
- This is intentional in the current design.

Example:

- Callback `cb` subscribed to `audio/*`
- Callback `cb` also subscribed to `audio/volume`
- Publishing `audio/volume`

Result:

- `cb` is called twice.
- The exact call order follows current LIFO traversal rules.

Callers must avoid overlapping subscriptions or make the callback idempotent
when repeated execution would be harmful.

### Ordering example

Assume the following registration order:

1. Register `audio/volume`
2. Register `audio/*`

Then assume the following subscription order:

1. Subscribe callback `cb_a` to `audio/volume`
2. Subscribe callback `cb_b` to `audio/volume`
3. Subscribe callback `cb_c` to `audio/*`

Publishing `audio/volume` executes callbacks in this order:

1. `cb_c` from `audio/*`
2. `cb_b` from `audio/volume`
3. `cb_a` from `audio/volume`

Reason:

- Topic node `audio/*` was registered later, so it is traversed first.
- Inside `audio/volume`, `cb_b` was subscribed later, so it runs before `cb_a`.

## Sync And Async Delivery

### Synchronous subscribers

Synchronous callbacks execute inline inside `event_bus_publish()`.

- They run in the publisher's context.
- They receive the original payload pointer.
- There is no isolation between synchronous callbacks.

That means one blocking or faulty synchronous callback can affect later
subscribers in the same publish path.

### Asynchronous subscribers

Asynchronous delivery uses a heap-backed `T_EVENT_BUS_ASYNC_EVENT`.

- The published topic string is deep-copied.
- The payload buffer is deep-copied when `data != NULL` and `data_len > 0`.
- `callback` is copied by value into the async event.
- The async sender owns the event only after it returns `true`.
- If queueing fails, the module frees the event immediately.

This design is required because watch task queues only copy the outer message
structure and do not deep-copy the payload pointed to by `u.buf`.

### Generic async contract

The event_bus module no longer relies on built-in task-specific async helpers.
Instead, each asynchronous subscription provides its own `T_EVENT_BUS_ASYNC_SEND`
transport function.

- The publish side prepares a `T_EVENT_BUS_ASYNC_EVENT`.
- The user-provided transport function sends it to the target task.
- The receiving task eventually calls `event_bus_async_dispatch()`.

In the current watch app, `IO_MSG_TYPE_EVENT_BUS` is one concrete example of
such a receiving-side dispatch path.

## Observability

When `EVENT_BUS_LOG_ENABLE` is enabled, event_bus emits graded logs.

- `ERR`: callback execution error.
- `WRN`: async drop due to allocation failure or queue send failure.
- `INF`: init, subscribe, unsubscribe, and other lifecycle logs.
- `DBG`: enqueue and dispatch traces with current pending depth.

The module also tracks these internal counters for async behavior observation:

- async queue success count
- async dispatch count
- async allocation failure count
- async send failure count
- current async pending count
- peak async pending count

## Usage Flow

Typical usage order:

1. Call `event_bus_init()`.
2. Register each exact or wildcard topic with `event_bus_topic_register()`.
3. Subscribe with `event_bus_subscribe()` or `event_bus_subscribe_async()`.
4. Publish with `event_bus_publish()`.
5. Remove subscriptions when needed.

## Current Non-goals

The following behaviors are not implemented in the current version.

- Automatic parent topic matching.
- `**` recursive wildcard syntax.
- Callback deduplication.
- FIFO ordering across topics or subscribers.
- Fault isolation for synchronous callbacks.
- Automatic cancellation of queued async events after unsubscribe.