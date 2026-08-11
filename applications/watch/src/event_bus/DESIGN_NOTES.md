# App Event Bus Design Notes

## Purpose

This document captures the design decisions, explored edge cases, and current
risk boundaries that came out of the event_bus review and refactor work.

It is intentionally more discussion-oriented than [README.md](README.md), which
keeps the public specification concise.

## Current Topic Decisions

### Full-path topic model

- Every topic is one complete string.
- `/` expresses hierarchy inside the same string.
- Topic depth is unbounded.

Examples:

- `audio`
- `audio/volume`
- `audio/state/mode`

The design does not split topic into separate main and subtopic fields.

### Exact match semantics

- `audio` is independent from `audio/volume`.
- Publishing `audio/volume` does not match `audio`.
- Publishing `audio/state/mode` does not match `audio/state`.

This avoids accidental parent-topic fan-out and keeps routing predictable.

### Wildcard semantics

Only one wildcard form is currently supported: terminal `/*`.

- `audio/*` matches `audio/volume`.
- `audio/*` matches `audio/state/mode`.
- `audio/*` does not match `audio`.

The more recursive `**` form is intentionally not implemented.

## Registration And Ordering Decisions

### Strict registration

- Topics must be registered before subscribe.
- `event_bus_subscribe()` and `event_bus_subscribe_async()` fail if the topic was
  not registered.
- Wildcard topics are also explicit nodes, so `audio/*` must be registered on
  its own if wildcard delivery is needed.

### LIFO traversal

Both topic nodes and subscriber nodes use head insertion.

Consequences:

- Later-registered topics are visited first.
- Later-subscribed callbacks on the same topic run first.

This is a deliberate property of the current linked-list implementation, not a
FIFO design.

### Duplicate callback policy

The module does not deduplicate callbacks.

- The same callback can be attached to multiple topics.
- The same callback can also be attached multiple times to the same topic.
- One publish can invoke the same callback more than once when multiple
  subscriptions match.

Callers must either avoid overlapping registrations or make the callback safe to
run more than once for one logical event.

## Async Delivery Notes

### Generic async transport

The current implementation uses a generic `T_EVENT_BUS_ASYNC_SEND` hook rather
than hardwiring app-task or wifi-task helper APIs into event_bus itself.

Flow:

1. `event_bus_publish()` matches a subscriber.
2. event_bus allocates and fills `T_EVENT_BUS_ASYNC_EVENT`.
3. The user-provided async transport sends that heap object to the target task.
4. The receiving task calls `event_bus_async_dispatch()`.

### Current app-side dispatch pattern

In the current watch app, one concrete receive path is:

```c
void watch_handle_io_message(T_IO_MSG *p_watch_msg)
{
  switch (p_watch_msg->subtype)
  {
  case IO_MSG_TYPE_EVENT_BUS:
    event_bus_async_dispatch((T_EVENT_BUS_ASYNC_EVENT *)p_watch_msg->u.buf);
    break;

  default:
    break;
  }
}
```

The sender side can use any transport wrapper that obeys the ownership rule:
return `true` only after the queue has accepted the `async_event` pointer.

## Memory And Lifetime Risks

### Heap fan-out cost

Assume one publish on topic `app/volume` with a payload length of 128 bytes,
and assume it matches two asynchronous subscribers.

For each matched async subscriber, event_bus allocates:

- one `T_EVENT_BUS_ASYNC_EVENT`
- one topic copy with length `strlen(topic) + 1`
- one payload copy with length `data_len`

Total temporary heap cost:

`2 * (sizeof(T_EVENT_BUS_ASYNC_EVENT) + strlen("app/volume") + 1 + 128)`

plus allocator overhead.

That means async fan-out grows linearly with subscriber count and payload size.

### Fragmentation and backlog

- High-frequency async publish creates repeated small malloc/free cycles.
- Large payloads are copied once per async subscriber.
- Queue backlog keeps copied topic and payload buffers alive longer.
- Peak heap usage therefore depends on both publish rate and consumer drain
  latency.

### Unsubscribe does not revoke queued async events

Unsubscribe only affects future matching during publish.

- It does not cancel async events that were already queued.
- A callback already copied into queued async events remains live from the
  queue's point of view.

This is the most important lifetime rule for business callers.

## `event_bus_unsubscribe_by_topic()` Review Notes

The current implementation removes the first matching callback on one exact
topic.

Important boundaries:

- It does not remove all duplicate subscriptions.
- If the same callback was subscribed multiple times on the same topic with
  different `user_data`, the removed entry depends on current list order.
- If business code assumes all matching registrations were removed and then
  frees a shared context object, a remaining subscription may still reference
  that object later.

### Async interaction

Even after `event_bus_unsubscribe_by_topic()` frees the subscriber node, queued
async events created earlier can still execute later because they already hold
their own copied `callback` and `user_data` fields.

### What this function does not do

- It does not cancel queued work.
- It does not remove all duplicates.
- It does not make concurrent publish safe.

## Concurrency Boundary

The current implementation does not provide full internal locking around topic
and subscriber list traversal.

### Lower-risk operating model

The design is much safer when:

- topic registration happens during init
- subscriptions are mostly established during init
- runtime unsubscribe is rare

Under that operating model, the main runtime concerns shift toward:

- callback reentrancy
- sync and async mixed ordering
- allocator pressure
- queue backlog

### Unsafe concurrent mutation

If `event_bus_publish()` races with `event_bus_unsubscribe()` or
`event_bus_unsubscribe_by_topic()`, one side can still traverse a subscriber node
while the other side frees it.

That remains a use-after-free hazard unless higher-level serialization or
additional locking is introduced.

## Observability Notes

When `EVENT_BUS_LOG_ENABLE` is enabled, the implementation exposes useful
runtime signals:

- `ERR`: callback execution error
- `WRN`: async allocation failure or queue send failure
- `INF`: lifecycle events such as init, subscribe, unsubscribe
- `DBG`: enqueue and dispatch traces with pending depth information

The module also tracks counters for:

- async queue success count
- async dispatch count
- async allocation failure count
- async send failure count
- current async pending count
- peak async pending count

## Current Non-goals

The following behaviors are intentionally out of scope for the current design:

- automatic parent-topic matching
- `**` recursive wildcard syntax
- callback deduplication
- FIFO ordering across topics or subscribers
- fault isolation for synchronous callbacks
- automatic cancellation of queued async events after unsubscribe