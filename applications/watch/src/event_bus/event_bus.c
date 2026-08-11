/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>
#include "event_bus.h"

#if defined(CONFIG_EVENT_BUS_LOG)
LOG_MODULE_REGISTER(event_bus, CONFIG_EVENT_BUS_LOG_LEVEL);
#else
LOG_MODULE_REGISTER(event_bus, 0);
#endif

#define EVENT_BUS_LOG_ERROR(fmt, ...) LOG_ERR("[event_bus] " fmt, ##__VA_ARGS__)
#define EVENT_BUS_LOG_WARN(fmt, ...)  LOG_WRN("[event_bus] " fmt, ##__VA_ARGS__)
#define EVENT_BUS_LOG_INFO(fmt, ...)  LOG_INF("[event_bus] " fmt, ##__VA_ARGS__)
#define EVENT_BUS_LOG_DEBUG(fmt, ...) LOG_DBG("[event_bus] " fmt, ##__VA_ARGS__)
#define EVENT_BUS_LOG(fmt, ...)       LOG_INF("[event_bus] " fmt, ##__VA_ARGS__)

/*============================================================================*
 *                           Types
 *============================================================================*/

typedef struct
{
    uint32_t async_queue_success_count;
    uint32_t async_dispatch_count;
    uint32_t async_alloc_fail_count;
    uint32_t async_send_fail_count;
    uint32_t async_pending_count;
    uint32_t async_pending_peak;
} T_EVENT_BUS_STATS;

/*============================================================================*
 *                            Variables
 *============================================================================*/

/* Head of the registered topic list. New topics are inserted at the front. */
static T_EVENT_BUS_TOPIC_NODE *s_topic_list = NULL;
static bool s_initialized = false;

static T_EVENT_BUS_STATS s_stats = {0};

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
 * @brief Duplicates a C string using module-owned heap memory.
 *
 * @param[in] src Source string.
 *
 * @return Newly allocated string, or NULL on allocation failure.
 */
static char *local_strdup(const char *src)
{
    if (src == NULL) { return NULL; }
    size_t len = strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (dst != NULL)
    {
        memcpy(dst, src, len);
    }
    return dst;
}

/**
 * @brief Duplicates a raw memory buffer using module-owned heap memory.
 *
 * @param[in] src Source buffer.
 * @param[in] len Buffer length in bytes.
 *
 * @return Newly allocated buffer, or NULL when src is NULL, len is 0, or the
 *         allocation fails.
 */
static void *local_memdup(const void *src, uint32_t len)
{
    void *dst;

    if (src == NULL || len == 0)
    {
        return NULL;
    }

    dst = malloc(len);
    if (dst != NULL)
    {
        memcpy(dst, src, len);
    }

    return dst;
}

static void event_bus_stats_reset(void)
{
    memset(&s_stats, 0, sizeof(s_stats));
}

static void event_bus_stats_note_async_send_fail(T_EVENT_BUS_ASYNC_EVENT *async_event)
{
    s_stats.async_alloc_fail_count++;
    if (async_event != NULL)
    {
        EVENT_BUS_LOG_WARN("Async drop topic=%s len=%lu reason=no-mem alloc_fail=%lu",
                           async_event->event_data.topic,
                           (unsigned long)async_event->event_data.data_len,
                           (unsigned long)s_stats.async_alloc_fail_count);
    }
}

static void event_bus_stats_note_async_enqueued(T_EVENT_BUS_ASYNC_EVENT *async_event)
{
    s_stats.async_queue_success_count++;
    s_stats.async_pending_count++;

    if (s_stats.async_pending_count > s_stats.async_pending_peak)
    {
        s_stats.async_pending_peak = s_stats.async_pending_count;
    }
    if (async_event != NULL)
    {
        EVENT_BUS_LOG_DEBUG("Async queued topic=%s len=%lu pending=%lu peak=%lu",
                            async_event->event_data.topic,
                            (unsigned long)async_event->event_data.data_len,
                            (unsigned long)s_stats.async_pending_count,
                            (unsigned long)s_stats.async_pending_peak);
    }
}

static void event_bus_stats_note_async_dispatched(T_EVENT_BUS_ASYNC_EVENT *async_event)
{
    s_stats.async_dispatch_count++;

    if (s_stats.async_pending_count > 0)
    {
        s_stats.async_pending_count--;
    }
    if (async_event != NULL)
    {
        EVENT_BUS_LOG_DEBUG("Async dispatch topic=%s len=%lu pending=%lu, async_event=%p, callback=%p, data_copy=%p, topic_copy=%p",
                            async_event->event_data.topic,
                            (unsigned long)async_event->event_data.data_len,
                            (unsigned long)s_stats.async_pending_count,
                            async_event, async_event->callback,
                            async_event->data_copy, async_event->topic_copy);
    }

}

/**
 * @brief Checks whether two topic names are exactly equal.
 */
static bool topic_name_equals(const char *lhs, const char *rhs)
{
    if (lhs == NULL || rhs == NULL)
    {
        return lhs == rhs;
    }

    return strcmp(lhs, rhs) == 0;
}

/**
 * @brief Checks whether a topic is a descendant wildcard of the form
 *        "path/" followed by "*".
 */
static bool topic_is_wildcard(const char *topic)
{
    size_t len;

    if (topic == NULL)
    {
        return false;
    }

    len = strlen(topic);
    return len >= 2 && topic[len - 2] == '/' && topic[len - 1] == '*';
}

/**
 * @brief Evaluates whether a subscription topic matches a published topic.
 *
 * Exact matches are supported. Wildcards are limited to a terminal "/"
 * followed by "*" suffix, which matches descendants only. Parent topics do not automatically
 * match child topics.
 */
static bool topic_matches_subscription(const char *subscribed_topic, const char *published_topic)
{
    size_t prefix_len;

    EVENT_BUS_LOG_DEBUG("Matching published topic '%s' against subscription '%s'",
                        published_topic, subscribed_topic);
    if (subscribed_topic == NULL || published_topic == NULL)
    {
        EVENT_BUS_LOG_DEBUG("No match: one of the topics is NULL");
        return false;
    }

    if (strcmp(subscribed_topic, published_topic) == 0)
    {
        EVENT_BUS_LOG_DEBUG("Exact match");
        return true;
    }

    if (!topic_is_wildcard(subscribed_topic))
    {
        EVENT_BUS_LOG_DEBUG("No match: subscription is not a wildcard and does not exactly match");
        return false;
    }

    EVENT_BUS_LOG_DEBUG("Subscription is a wildcard, checking prefix");
    prefix_len = strlen(subscribed_topic) - 1;
    EVENT_BUS_LOG_DEBUG("Comparing prefix '%.*s' against published topic '%s'", (int)prefix_len,
                        subscribed_topic, published_topic);
    return strncmp(subscribed_topic, published_topic, prefix_len) == 0 &&
           published_topic[prefix_len] != '\0';
}

/**
 * @brief Finds a topic node by exact topic string.
 */
static T_EVENT_BUS_TOPIC_NODE *topic_find_exact(const char *topic)
{
    T_EVENT_BUS_TOPIC_NODE *node = s_topic_list;

    while (node != NULL)
    {
        if (topic_name_equals(node->topic, topic))
        {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

/**
 * @brief Finds a topic node by full topic path.
 */
static T_EVENT_BUS_TOPIC_NODE *topic_find_by_full(const char *full_topic)
{
    return topic_find_exact(full_topic);
}

/**
 * @brief Creates and registers a new topic node.
 *
 * The new node is inserted at the head of s_topic_list, so the most recently
 * registered topic is traversed first during publish.
 */
static int32_t topic_create(const char *topic,
                            T_EVENT_BUS_TOPIC_NODE **out_node)
{
    T_EVENT_BUS_TOPIC_NODE *node = (T_EVENT_BUS_TOPIC_NODE *)malloc(sizeof(
                                                                        T_EVENT_BUS_TOPIC_NODE));
    if (node == NULL)
    {
        EVENT_BUS_LOG_ERROR("Out of memory");
        return EVENT_BUS_ERR_NO_MEM;
    }

    memset(node, 0, sizeof(T_EVENT_BUS_TOPIC_NODE));

    node->topic = local_strdup(topic);
    if (node->topic == NULL)
    {
        free(node);
        EVENT_BUS_LOG_ERROR("Out of memory");
        return EVENT_BUS_ERR_NO_MEM;
    }

    node->next = s_topic_list;
    s_topic_list = node;

    *out_node = node;
    return EVENT_BUS_OK;
}

/**
 * @brief Frees a topic node and all subscribers linked to it.
 */
static void topic_free(T_EVENT_BUS_TOPIC_NODE *node)
{
    T_EVENT_BUS_SUBSCRIBER *subscriber;
    T_EVENT_BUS_SUBSCRIBER *next;

    subscriber = node->subscribers;
    while (subscriber != NULL)
    {
        next = subscriber->next;
        free(subscriber);
        subscriber = next;
    }

    if (node->topic != NULL)
    {
        free(node->topic);
    }
    free(node);
}

/**
 * @brief Allocates a subscriber node for a topic.
 */
static T_EVENT_BUS_SUBSCRIBER *subscriber_create(T_EVENT_BUS_TOPIC_NODE *topic,
                                                 T_EVENT_BUS_CALLBACK callback,
                                                 bool is_async,
                                                 T_EVENT_BUS_ASYNC_SEND async_send,
                                                 void *async_context)
{
    T_EVENT_BUS_SUBSCRIBER *subscriber;

    subscriber = (T_EVENT_BUS_SUBSCRIBER *)malloc(sizeof(T_EVENT_BUS_SUBSCRIBER));
    if (subscriber == NULL)
    {
        return NULL;
    }

    memset(subscriber, 0, sizeof(T_EVENT_BUS_SUBSCRIBER));
    subscriber->callback = callback;
    subscriber->is_async = is_async;
    subscriber->async_send = async_send;
    subscriber->async_context = async_context;
    subscriber->topic = topic;

    return subscriber;
}

/**
 * @brief Inserts a subscriber at the head of a topic's subscriber list.
 *
 * This preserves LIFO execution order inside the same topic node.
 */
static void subscriber_append(T_EVENT_BUS_TOPIC_NODE *topic,
                              T_EVENT_BUS_SUBSCRIBER *subscriber)
{
    subscriber->next = topic->subscribers;
    topic->subscribers = subscriber;
    topic->subscriber_count++;
}

/**
 * @brief Frees a queued asynchronous event object.
 */
static void event_bus_async_event_free(T_EVENT_BUS_ASYNC_EVENT *async_event)
{
    if (async_event == NULL)
    {
        return;
    }

    if (async_event->topic_copy != NULL)
    {
        free(async_event->topic_copy);
    }

    if (async_event->data_copy != NULL)
    {
        free(async_event->data_copy);
    }

    free(async_event);
}

/**
 * @brief Builds a heap-backed asynchronous event for cross-task delivery.
 *
 * Both the topic string and payload are deep-copied because the watch task
 * queues only keep the outer message struct, not the pointed-to payload.
 */
static T_EVENT_BUS_ASYNC_EVENT *event_bus_async_event_create(const
                                                             T_EVENT_BUS_SUBSCRIBER *subscriber,
                                                             const char *full_topic,
                                                             void *data,
                                                             uint32_t data_len)
{
    T_EVENT_BUS_ASYNC_EVENT *async_event;

    async_event = (T_EVENT_BUS_ASYNC_EVENT *)malloc(sizeof(T_EVENT_BUS_ASYNC_EVENT));
    if (async_event == NULL)
    {
        return NULL;
    }

    memset(async_event, 0, sizeof(T_EVENT_BUS_ASYNC_EVENT));
    async_event->callback = subscriber->callback;
    async_event->topic_copy = local_strdup(full_topic);
    if (async_event->topic_copy == NULL)
    {
        event_bus_async_event_free(async_event);
        return NULL;
    }

    if (data != NULL && data_len > 0)
    {
        async_event->data_copy = local_memdup(data, data_len);
        if (async_event->data_copy == NULL)
        {
            event_bus_async_event_free(async_event);
            return NULL;
        }
    }

    async_event->event_data.topic = async_event->topic_copy;
    async_event->event_data.data = async_event->data_copy;
    async_event->event_data.data_len = data_len;

    EVENT_BUS_LOG_DEBUG("Created async event for topic '%s' with data length %lu, async_event %p, data_copy %p, topic_copy %p",
                        async_event->event_data.topic, (unsigned long)async_event->event_data.data_len, async_event,
                        async_event->data_copy, async_event->topic_copy);

    return async_event;
}

/**
 * @brief Delivers one publish operation to all subscribers on a matched topic.
 */
static void publish_to_topic(T_EVENT_BUS_TOPIC_NODE *node,
                             const char *published_topic,
                             void *data, uint32_t data_len)
{
    T_EVENT_BUS_SUBSCRIBER *subscriber;
    T_EVENT_BUS_SUBSCRIBER *next;

    if (node == NULL || node->subscriber_count == 0)
    {
        return;
    }

    EVENT_BUS_LOG("Publish to topic %s", published_topic);

    subscriber = node->subscribers;
    while (subscriber != NULL)
    {
        next = subscriber->next;

        if (subscriber->is_async)
        {
            T_EVENT_BUS_ASYNC_EVENT *async_event;

            async_event = event_bus_async_event_create(subscriber, published_topic, data, data_len);
            if (async_event == NULL)
            {
                event_bus_stats_note_async_send_fail(async_event);
            }
            else if (subscriber->async_send == NULL ||
                     subscriber->async_send(async_event, subscriber->async_context) != EVENT_BUS_OK)
            {
                event_bus_stats_note_async_send_fail(async_event);
                event_bus_async_event_free(async_event);
            }
            else
            {
                event_bus_stats_note_async_enqueued(async_event);
            }
        }
        else if (subscriber->callback != NULL)
        {
            T_EVENT_BUS_EVENT_DATA event_data =
            {
                .topic = published_topic,
                .data = data,
                .data_len = data_len
            };
            /* Sync subscribers run inline in publish context without isolation. */
            int32_t cb_ret = subscriber->callback(&event_data);
            if (cb_ret != EVENT_BUS_OK)
            {
                EVENT_BUS_LOG("Subscriber callback error %d on topic %s", cb_ret, published_topic);
            }
        }

        subscriber = next;
    }
}


/*============================================================================*
 *                           Public Functions
 *============================================================================*/

/**
 * @brief Initializes the event bus runtime state.
 *
 * This routine resets the internal topic list and runtime statistics.
 * Calling it again after successful initialization is harmless.
 */
int32_t event_bus_init()
{
    if (s_initialized)
    {
        return EVENT_BUS_OK;
    }

    s_topic_list = NULL;
    event_bus_stats_reset();
    s_initialized = true;

    EVENT_BUS_LOG("Initialized");

    return EVENT_BUS_OK;
}

/**
 * @brief Subscribes to a registered topic synchronously.
 *
 * @param[out] handle Subscription handle returned on success.
 * @param[in] topic Registered topic string.
 * @param[in] callback Callback executed in the publisher context.
 */
int32_t event_bus_subscribe(T_EVENT_BUS_SUBSCRIBER_HANDLE *handle, const char *topic,
                            T_EVENT_BUS_CALLBACK callback)
{
    T_EVENT_BUS_TOPIC_NODE *node;
    T_EVENT_BUS_SUBSCRIBER *subscriber;

    if (!s_initialized)
    {
        return EVENT_BUS_ERR_NOT_INIT;
    }

    if (callback == NULL || handle == NULL || topic == NULL)
    {
        EVENT_BUS_LOG_ERROR("Invalid parameters for subscription: handle=%p, topic=%p, callback=%p", handle,
                            topic, callback);
        return EVENT_BUS_ERR_INVALID_PARAM;
    }

    node = topic_find_exact(topic);
    if (node == NULL)
    {
        EVENT_BUS_LOG_ERROR("Topic '%s' not found for subscription, check if topic is registered as wildcard",
                            topic);
        return EVENT_BUS_ERR_TOPIC_NOT_FOUND;
    }

    subscriber = subscriber_create(node, callback, false, NULL, NULL);
    if (subscriber == NULL)
    {
        EVENT_BUS_LOG_ERROR("Out of memory");
        return EVENT_BUS_ERR_NO_MEM;
    }

    subscriber_append(node, subscriber);
    *handle = (T_EVENT_BUS_SUBSCRIBER_HANDLE)subscriber;

    EVENT_BUS_LOG("Subscribed to topic %s (sync)", topic);

    return EVENT_BUS_OK;
}

/**
 * @brief Subscribes to a registered topic asynchronously.
 *
 * The event bus deep-copies the topic and payload before forwarding the
 * request through the transport hook.
 *
 * @param[out] handle Subscription handle returned on success.
 * @param[in] topic Registered topic string.
 * @param[in] async_send Transport hook used to queue the async event.
 * @param[in] async_context Context pointer passed to async_send().
 * @param[in] callback Callback executed in the receiver context.
 */
int32_t event_bus_subscribe_async(T_EVENT_BUS_SUBSCRIBER_HANDLE *handle, const char *topic,
                                  T_EVENT_BUS_ASYNC_SEND async_send, void *async_context,
                                  T_EVENT_BUS_CALLBACK callback)
{
    T_EVENT_BUS_TOPIC_NODE *node;
    T_EVENT_BUS_SUBSCRIBER *subscriber;

    if (!s_initialized)
    {
        return EVENT_BUS_ERR_NOT_INIT;
    }

    if (async_send == NULL || handle == NULL || topic == NULL || callback == NULL)
    {
        return EVENT_BUS_ERR_INVALID_PARAM;
    }

    node = topic_find_exact(topic);
    if (node == NULL)
    {
        return EVENT_BUS_ERR_TOPIC_NOT_FOUND;
    }

    subscriber = subscriber_create(node, callback, true, async_send, async_context);
    if (subscriber == NULL)
    {
        EVENT_BUS_LOG_ERROR("Out of memory");
        return EVENT_BUS_ERR_NO_MEM;
    }

    subscriber_append(node, subscriber);
    *handle = (T_EVENT_BUS_SUBSCRIBER_HANDLE)subscriber;

    EVENT_BUS_LOG("Subscribed to topic %s (async)", topic);

    return EVENT_BUS_OK;
}

/**
 * @brief Removes a subscriber by handle.
 *
 * @param[in] handle Subscription handle returned by subscribe APIs.
 */
int32_t event_bus_unsubscribe(T_EVENT_BUS_SUBSCRIBER_HANDLE handle)
{
    T_EVENT_BUS_SUBSCRIBER *subscriber;
    T_EVENT_BUS_TOPIC_NODE *topic;
    T_EVENT_BUS_SUBSCRIBER **link;

    if (!s_initialized)
    {
        return EVENT_BUS_ERR_NOT_INIT;
    }

    if (handle == NULL)
    {
        return EVENT_BUS_ERR_INVALID_PARAM;
    }

    subscriber = (T_EVENT_BUS_SUBSCRIBER *)handle;

    topic = s_topic_list;
    while (topic != NULL)
    {
        link = &topic->subscribers;
        while (*link != NULL)
        {
            if (*link == subscriber)
            {
                *link = subscriber->next;
                if (topic->subscriber_count > 0)
                {
                    topic->subscriber_count--;
                }

                subscriber->topic = NULL;
                free(subscriber);

                EVENT_BUS_LOG("Unsubscribed handle");

                return EVENT_BUS_OK;
            }

            link = &(*link)->next;
        }

        topic = topic->next;
    }

    return EVENT_BUS_ERR_SUBSCRIBER_NOT_FOUND;
}

/**
 * @brief Removes the first callback match on an exact topic.
 *
 * @param[in] topic Registered topic string.
 * @param[in] callback Callback to remove.
 */
int32_t event_bus_unsubscribe_by_topic(const char *topic,
                                       T_EVENT_BUS_CALLBACK callback)
{
    T_EVENT_BUS_TOPIC_NODE *node;
    T_EVENT_BUS_SUBSCRIBER **link;

    if (!s_initialized)
    {
        return EVENT_BUS_ERR_NOT_INIT;
    }

    if (callback == NULL || topic == NULL)
    {
        return EVENT_BUS_ERR_INVALID_PARAM;
    }

    node = topic_find_by_full(topic);
    if (node == NULL)
    {
        return EVENT_BUS_ERR_TOPIC_NOT_FOUND;
    }

    link = &node->subscribers;
    while (*link != NULL)
    {
        if ((*link)->callback == callback)
        {
            T_EVENT_BUS_SUBSCRIBER *subscriber = *link;

            *link = subscriber->next;
            if (node->subscriber_count > 0)
            {
                node->subscriber_count--;
            }
            subscriber->topic = NULL;
            free(subscriber);

            EVENT_BUS_LOG("Unsubscribed from topic %s", topic);

            return EVENT_BUS_OK;
        }

        link = &(*link)->next;
    }

    return EVENT_BUS_ERR_SUBSCRIBER_NOT_FOUND;
}

/**
 * @brief Publishes a payload to all matching topic nodes.
 *
 * Exact topic matches are supported. Wildcard topics that use a trailing
 * slash-star suffix
 * match descendant topics only.
 *
 * @param[in] topic Full published topic string.
 * @param[in] data Optional payload pointer.
 * @param[in] data_len Payload length in bytes.
 */
int32_t event_bus_publish(const char *topic, void *data, uint32_t data_len)
{
    T_EVENT_BUS_TOPIC_NODE *node;
    bool delivered = false;

    EVENT_BUS_LOG_DEBUG("Publishing to topic '%s'", topic);
    if (!s_initialized)
    {
        EVENT_BUS_LOG_DEBUG("Event bus not initialized");
        return EVENT_BUS_ERR_NOT_INIT;
    }

    if (topic == NULL)
    {
        EVENT_BUS_LOG_DEBUG("Invalid topic parameter");
        return EVENT_BUS_ERR_INVALID_PARAM;
    }

    /* s_topic_list is the head pointer, not the tail pointer. */
    node = s_topic_list;
    EVENT_BUS_LOG_DEBUG("Starting publish traversal for topic '%s' node %p from head of topic list",
                        topic, node);
    while (node != NULL)
    {
        EVENT_BUS_LOG_DEBUG("Checking topic node '%s' against published topic '%s'",
                            node->topic, topic);
        if (topic_matches_subscription(node->topic, topic))
        {
            publish_to_topic(node, topic, data, data_len);
            delivered = true;
        }

        node = node->next;
    }

    return delivered ? EVENT_BUS_OK : EVENT_BUS_ERR_TOPIC_NOT_FOUND;
}

/**
 * @brief Gets the subscriber count for an exact topic.
 *
 * @param[in] topic Registered topic string.
 * @param[out] count Subscriber count for the exact topic.
 */
int32_t event_bus_get_subscriber_count(const char *topic, uint8_t *count)
{
    T_EVENT_BUS_TOPIC_NODE *node = topic_find_by_full(topic);
    if (node != NULL)
    {
        *count = node->subscriber_count;
    }
    else
    {
        *count = 0;
    }
    return EVENT_BUS_OK;
}

/**
 * @brief Registers a topic before subscriptions are added.
 *
 * Registering the same topic more than once is harmless.
 *
 * @param[in] topic Full topic string to register.
 */
int32_t event_bus_topic_register(const char *topic)
{
    if (!s_initialized)
    {
        return EVENT_BUS_ERR_NOT_INIT;
    }

    if (topic == NULL)
    {
        return EVENT_BUS_ERR_INVALID_PARAM;
    }

    T_EVENT_BUS_TOPIC_NODE *node;

    node = topic_find_exact(topic);
    if (node != NULL)
    {
        EVENT_BUS_LOG("Topic %s already registered", topic);
        return EVENT_BUS_OK;
    }

    int32_t ret = topic_create(topic, &node);
    if (ret == EVENT_BUS_OK)
    {
        EVENT_BUS_LOG("Topic registered: %s", topic);
    }
    return ret;
}

/**
 * @brief Dispatches one queued async event and frees it afterward.
 *
 * @param[in] async_event Heap-backed async event object.
 */
void event_bus_async_dispatch(T_EVENT_BUS_ASYNC_EVENT *async_event)
{
    if (async_event == NULL)
    {
        return;
    }

    event_bus_stats_note_async_dispatched(async_event);

    if (async_event->callback != NULL)
    {
        int32_t cb_ret = async_event->callback(&async_event->event_data);
        if (cb_ret != EVENT_BUS_OK)
        {
            EVENT_BUS_LOG("Subscriber callback error %d on topic %s", cb_ret, async_event->event_data.topic);
        }
    }

    event_bus_async_event_free(async_event);
}