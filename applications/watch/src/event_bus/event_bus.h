/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _EVENT_BUS_H_
#define _EVENT_BUS_H_

/*============================================================================*
 *                        Header Files
 *============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                         Constants
 *============================================================================*/


/*============================================================================*
 *                         Types
 *============================================================================*/

typedef void *T_EVENT_BUS_SUBSCRIBER_HANDLE;

/**
 * @brief App event bus API return codes.
 */
typedef enum
{
    EVENT_BUS_OK = 0,
    EVENT_BUS_ERR_NO_MEM,
    EVENT_BUS_ERR_TOPIC_NOT_FOUND,
    EVENT_BUS_ERR_SUBSCRIBER_NOT_FOUND,
    EVENT_BUS_ERR_INVALID_PARAM,
    EVENT_BUS_ERR_FULL,
    EVENT_BUS_ERR_NOT_INIT,
} T_EVENT_BUS_ERR;

/**
 * @brief Event payload passed to app event bus callbacks.
 *
 * The topic field always contains the full published path. The data pointer
 * references the caller-owned payload for synchronous subscribers, and a
 * deep-copied payload for asynchronous subscribers.
 */
typedef struct T_EVENT_BUS_EVENT_DATA
{
    const char *topic;
    void *data;
    uint32_t data_len;
} T_EVENT_BUS_EVENT_DATA;

/**
 * @brief App event bus callback prototype.
 *
 * @param[in] event_data Published topic, payload, and payload length.
 *
 * @return Detailed EVENT_BUS_* error code in an int32_t value. Non-zero values
 *         are currently logged only by the asynchronous dispatch path.
 */
typedef int32_t(*T_EVENT_BUS_CALLBACK)(T_EVENT_BUS_EVENT_DATA *event_data);

/**
 * @brief Heap object queued across tasks for asynchronous delivery.
 */
typedef struct T_EVENT_BUS_ASYNC_EVENT
{
    T_EVENT_BUS_CALLBACK callback;
    T_EVENT_BUS_EVENT_DATA event_data;
    char *topic_copy;
    void *data_copy;
} T_EVENT_BUS_ASYNC_EVENT;

/**
 * @brief Transport hook used by asynchronous subscriptions.
 *
 * The sender takes ownership of async_event only when it returns EVENT_BUS_OK.
 * Otherwise the event_bus module frees the event before returning.
 *
 * @param[in] async_event Heap-allocated event object prepared by event_bus.
 * @param[in] context Caller-provided context pointer.
 *
 * @retval EVENT_BUS_OK  Event was accepted by the target task/queue.
 * @retval Other values Event was rejected and will be freed by event_bus.
 */
typedef int32_t(*T_EVENT_BUS_ASYNC_SEND)(T_EVENT_BUS_ASYNC_EVENT *async_event, void *context);

/**
 * @brief Runtime subscriber node stored in a topic's subscriber list.
 */
typedef struct T_EVENT_BUS_SUBSCRIBER
{
    T_EVENT_BUS_CALLBACK callback;
    bool is_async;
    T_EVENT_BUS_ASYNC_SEND async_send;
    void *async_context;
    struct T_EVENT_BUS_TOPIC_NODE *topic;
    struct T_EVENT_BUS_SUBSCRIBER *next;
} T_EVENT_BUS_SUBSCRIBER;

/**
 * @brief Runtime topic node stored in the global topic list.
 *
 * Topics are stored as full strings such as "audio/volume" or wildcard
 * descendants like "audio/" followed by "*".
 */
typedef struct T_EVENT_BUS_TOPIC_NODE
{
    char *topic;
    T_EVENT_BUS_SUBSCRIBER *subscribers;
    uint8_t subscriber_count;
    struct T_EVENT_BUS_TOPIC_NODE *next;
} T_EVENT_BUS_TOPIC_NODE;

/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
 * @brief Initializes the event bus runtime state.
 *
 * This routine resets the internal topic list and runtime statistics.
 * Calling it again after successful initialization is harmless.
 */
int32_t event_bus_init(void);

/**
 * @brief Subscribes to a registered topic synchronously.
 *
 * @param[out] handle Subscription handle returned on success.
 * @param[in] topic Registered topic string.
 * @param[in] callback Callback executed in the publisher context.
 */
int32_t event_bus_subscribe(T_EVENT_BUS_SUBSCRIBER_HANDLE *handle, const char *topic,
                            T_EVENT_BUS_CALLBACK callback);

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
                                  T_EVENT_BUS_CALLBACK callback);

/**
 * @brief Removes a subscriber by handle.
 *
 * @param[in] handle Subscription handle returned by subscribe APIs.
 */
int32_t event_bus_unsubscribe(T_EVENT_BUS_SUBSCRIBER_HANDLE handle);

/**
 * @brief Removes the first callback match on an exact topic.
 *
 * @param[in] topic Registered topic string.
 * @param[in] callback Callback to remove.
 */
int32_t event_bus_unsubscribe_by_topic(const char *topic,
                                       T_EVENT_BUS_CALLBACK callback);

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
int32_t event_bus_publish(const char *topic, void *data, uint32_t data_len);

/**
 * @brief Gets the subscriber count for an exact topic.
 *
 * @param[in] topic Registered topic string.
 * @param[out] count Subscriber count for the exact topic.
 */
int32_t event_bus_get_subscriber_count(const char *topic, uint8_t *count);

/**
 * @brief Registers a topic before subscriptions are added.
 *
 * @param[in] topic Full topic string to register.
 *
 * Registering the same topic more than once is harmless.
 */
int32_t event_bus_topic_register(const char *topic);

/**
 * @brief Dispatches one queued async event and frees it afterward.
 *
 * @param[in] async_event Heap-backed async event object.
 */
void event_bus_async_dispatch(T_EVENT_BUS_ASYNC_EVENT *async_event);

#ifdef __cplusplus
}
#endif /* _EVENT_BUS_H_ */

#endif