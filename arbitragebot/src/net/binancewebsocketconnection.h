#pragma once

#include "net/iconnection.h"
#include "moodycamel/concurrentqueue.h"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <string>

class BinanceConnection : public IConnection {
public:
    BinanceConnection();
    ~BinanceConnection();

    void connect(const std::string& uri) override;
    void disconnect() override;
    bool is_connected() const override;
    void send(const std::string& message, std::function<void(const std::string&)> callback = nullptr) override;

    void set_message_handler(MessageHandler handler) override;
    void set_open_handler(ConnectionHandler handler) override;
    void set_close_handler(ConnectionHandler handler) override;
    void set_fail_handler(ErrorHandler handler) override;

private:
    using Client = websocketpp::client<websocketpp::config::asio_tls_client>;
    using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

    void run_send_loop();
    void run_event_loop();

    Client m_client;
    websocketpp::connection_hdl m_hdl;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_connected{false};

    // Lock-free queues
    moodycamel::ConcurrentQueue<std::string> m_send_queue;

    // Threads
    std::thread m_thread_send;
    std::thread m_thread_event;

    // Handlers
    MessageHandler m_msg_handler;
    ConnectionHandler m_open_handler;
    ConnectionHandler m_close_handler;
    ErrorHandler m_fail_handler;
};
