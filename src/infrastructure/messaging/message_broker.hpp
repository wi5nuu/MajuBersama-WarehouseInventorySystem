#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>

#include "../../common/result.hpp"

namespace inventory {

/**
 * @brief Represents a message in the publish-subscribe system.
 */
struct Message {
    std::string topic;                                             ///< Logical channel name
    std::string payload;                                           ///< Serialized message body
    std::string message_id;                                        ///< Unique message identifier
    std::chrono::system_clock::time_point timestamp;               ///< When the message was created
};

/**
 * @brief Abstract interface for a publish-subscribe message broker.
 *
 * Supports topic-based publish/consume with optional subscription management.
 * All operations return Result<T> and are expected to be thread-safe.
 */
class MessageBroker {
public:
    virtual ~MessageBroker() = default;

    /**
     * @brief Publishes a message to all subscribers of the given topic.
     * @param msg The message to publish
     * @return Ok on success, Error on failure
     */
    [[nodiscard]] virtual Result<void> publish(const Message& msg) = 0;

    /**
     * @brief Consumes a single message from the given topic with an optional timeout.
     * @param topic The topic to consume from
     * @param timeout Maximum wait duration (default 100ms)
     * @return The message if available within timeout, std::nullopt if timed out, Error on failure
     */
    [[nodiscard]] virtual Result<std::optional<Message>> consume(
        const std::string& topic,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) = 0;

    /**
     * @brief Subscribes to a topic, creating the internal queue if needed.
     * @param topic The topic to subscribe to
     * @return Ok on success, Error on failure
     */
    [[nodiscard]] virtual Result<void> subscribe(const std::string& topic) = 0;

    /**
     * @brief Unsubscribes from a topic and removes its message queue.
     * @param topic The topic to unsubscribe from
     * @return Ok on success, Error on failure
     */
    [[nodiscard]] virtual Result<void> unsubscribe(const std::string& topic) = 0;
};

/**
 * @brief Thread-safe in-memory implementation of MessageBroker.
 *
 * Uses a per-topic std::queue protected by a single mutex.
 * The consume() method blocks up to the specified timeout using a condition variable.
 */
class InMemoryMessageBroker final : public MessageBroker {
public:
    InMemoryMessageBroker() = default;

    ~InMemoryMessageBroker() override;

    InMemoryMessageBroker(const InMemoryMessageBroker&) = delete;
    InMemoryMessageBroker& operator=(const InMemoryMessageBroker&) = delete;
    InMemoryMessageBroker(InMemoryMessageBroker&&) = delete;
    InMemoryMessageBroker& operator=(InMemoryMessageBroker&&) = delete;

    [[nodiscard]] Result<void> publish(const Message& msg) override;

    [[nodiscard]] Result<std::optional<Message>> consume(
        const std::string& topic,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) override;

    [[nodiscard]] Result<void> subscribe(const std::string& topic) override;

    [[nodiscard]] Result<void> unsubscribe(const std::string& topic) override;

    /**
     * @brief Returns the number of pending messages for a given topic.
     * @param topic The topic to query
     * @return Message count, or Error if topic not found
     */
    [[nodiscard]] Result<std::size_t> pending_count(const std::string& topic) const;

    /**
     * @brief Returns the list of currently subscribed topics.
     */
    [[nodiscard]] std::vector<std::string> subscribed_topics() const;

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::unordered_map<std::string, std::queue<Message>> m_queues;
    std::atomic<bool> m_running{ true };
};

} // namespace inventory
