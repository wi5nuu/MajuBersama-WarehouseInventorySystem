#include "message_broker.hpp"

#include <algorithm>
#include <format>
#include <random>
#include <vector>

namespace inventory {

InMemoryMessageBroker::~InMemoryMessageBroker()
{
    m_running = false;
    m_cv.notify_all();
}

Result<void> InMemoryMessageBroker::publish(const Message& msg)
{
    if (!m_running) {
        return Error::internal("MessageBroker is shut down");
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_queues.find(msg.topic);
        if (it == m_queues.end()) {
            return Error::not_found(
                std::format("No subscribers for topic '{}'", msg.topic));
        }

        Message enriched = msg;
        if (enriched.message_id.empty()) {
            static std::mt19937_64 rng(std::random_device{}());
            enriched.message_id = std::format("{:016x}{:016x}", rng(), rng());
        }
        if (enriched.timestamp == std::chrono::system_clock::time_point{}) {
            enriched.timestamp = std::chrono::system_clock::now();
        }

        it->second.push(std::move(enriched));
    }

    m_cv.notify_all();
    return {};
}

Result<std::optional<Message>> InMemoryMessageBroker::consume(
    const std::string& topic,
    std::chrono::milliseconds timeout)
{
    if (!m_running) {
        return Error::internal("MessageBroker is shut down");
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_queues.find(topic);
    if (it == m_queues.end()) {
        return Error::not_found(
            std::format("Topic '{}' not found", topic));
    }

    auto& queue = it->second;

    auto pred = [this, &queue]() {
        return !m_running || !queue.empty();
    };

    if (queue.empty()) {
        m_cv.wait_for(lock, timeout, pred);
    }

    if (!m_running) {
        return std::optional<Message>{ std::nullopt };
    }

    if (queue.empty()) {
        return std::optional<Message>{ std::nullopt };
    }

    Message msg = std::move(queue.front());
    queue.pop();
    return std::optional<Message>{ std::move(msg) };
}

Result<void> InMemoryMessageBroker::subscribe(const std::string& topic)
{
    if (topic.empty()) {
        return Error::validation("Topic name cannot be empty");
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_queues.find(topic) != m_queues.end()) {
        return Error::conflict(
            std::format("Already subscribed to topic '{}'", topic));
    }

    m_queues.emplace(topic, std::queue<Message>{});
    return {};
}

Result<void> InMemoryMessageBroker::unsubscribe(const std::string& topic)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_queues.find(topic);
    if (it == m_queues.end()) {
        return Error::not_found(
            std::format("Topic '{}' not found", topic));
    }

    m_queues.erase(it);
    return {};
}

Result<std::size_t> InMemoryMessageBroker::pending_count(const std::string& topic) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_queues.find(topic);
    if (it == m_queues.end()) {
        return Error::not_found(
            std::format("Topic '{}' not found", topic));
    }

    return it->second.size();
}

std::vector<std::string> InMemoryMessageBroker::subscribed_topics() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<std::string> topics;
    topics.reserve(m_queues.size());
    for (const auto& [topic, _] : m_queues) {
        topics.push_back(topic);
    }
    return topics;
}

} // namespace inventory
