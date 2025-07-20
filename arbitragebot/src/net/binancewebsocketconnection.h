#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include "net/iconnection.h"

class BinanceWebSocketConnection : public IConnection {
    using Client = websocketpp::client<websocketpp::config::asio_tls_client>;
    using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

public:
    BinanceWebSocketConnection();
    ~BinanceWebSocketConnection() override = default;

    void connect(const std::string& uri) override;
    void disconnect() override;
    bool is_connected() const override;
    void send(const std::string& message, std::function<void(const std::string&)> callback = nullptr) override;

    void poll();  // Chiamato esplicitamente nel loop principale

    void set_message_handler(MessageHandler handler) override;
    void set_open_handler(ConnectionHandler handler) override;
    void set_close_handler(ConnectionHandler handler) override;
    void set_fail_handler(ErrorHandler handler) override;

    void set_debug(bool enable);

private:
    Client m_client;
    websocketpp::connection_hdl m_connection;
    std::atomic<bool> m_is_connected;

    MessageHandler m_message_handler;
    ConnectionHandler m_open_handler;
    ConnectionHandler m_close_handler;
    ErrorHandler m_fail_handler;

    ContextPtr on_tls_init();
    void handle_message(websocketpp::connection_hdl, Client::message_ptr msg);
    void handle_open(websocketpp::connection_hdl hdl);
    void handle_close(websocketpp::connection_hdl hdl);
    void handle_fail(websocketpp::connection_hdl hdl);
};
