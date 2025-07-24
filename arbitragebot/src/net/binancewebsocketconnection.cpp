#include "net/binancewebsocketconnection.h"
#include <iostream>
#include <chrono>
#include <openssl/ssl.h>  


BinanceConnection::BinanceConnection() {
    m_client.clear_access_channels(websocketpp::log::alevel::all);
    m_client.init_asio();

    m_client.set_tls_init_handler([](websocketpp::connection_hdl) {
        ContextPtr ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
        ctx->set_options(asio::ssl::context::default_workarounds);
        return ctx;
    });

    m_client.set_open_handler([this](websocketpp::connection_hdl hdl) {
        m_hdl = hdl;
        m_connected = true;
        if (m_open_handler) m_open_handler();
    });

    m_client.set_close_handler([this](websocketpp::connection_hdl) {
        m_connected = false;
        if (m_close_handler) m_close_handler();
    });

    m_client.set_fail_handler([this](websocketpp::connection_hdl) {
        m_connected = false;
        if (m_fail_handler) m_fail_handler("Connection failed");
    });

    m_client.set_message_handler([this](websocketpp::connection_hdl, Client::message_ptr msg) {
        if (m_msg_handler) m_msg_handler(msg->get_payload());
    });
}

BinanceConnection::~BinanceConnection() {
    disconnect();
}

void BinanceConnection::connect(const std::string& uri) {
    websocketpp::lib::error_code ec;
    auto con = m_client.get_connection(uri, ec);
    if (ec) {
        if (m_fail_handler) m_fail_handler(ec.message());
        return;
    }

    m_hdl = con->get_handle();
    m_client.connect(con);
    m_running = true;

    m_thread_event = std::thread([this]() { m_client.run(); });
    m_thread_send = std::thread([this]() { run_send_loop(); });
}

void BinanceConnection::disconnect() {
    m_running = false;
    if (m_connected) {
        websocketpp::lib::error_code ec;
        m_client.close(m_hdl, websocketpp::close::status::normal, "", ec);
    }

    if (m_thread_event.joinable()) m_thread_event.join();
    if (m_thread_send.joinable()) m_thread_send.join();
}

bool BinanceConnection::is_connected() const {
    return m_connected.load();
}

void BinanceConnection::send(const std::string& message, std::function<void(const std::string&)> callback) {
    m_send_queue.enqueue(message);
}

void BinanceConnection::run_send_loop() {
    while (m_running) {
        std::string msg;
        if (m_send_queue.try_dequeue(msg)) {
            websocketpp::lib::error_code ec;
            m_client.send(m_hdl, msg, websocketpp::frame::opcode::text, ec);
            if (ec && m_fail_handler) m_fail_handler(ec.message());
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

// Setters
void BinanceConnection::set_message_handler(MessageHandler handler) {
    m_msg_handler = std::move(handler);
}
void BinanceConnection::set_open_handler(ConnectionHandler handler) {
    m_open_handler = std::move(handler);
}
void BinanceConnection::set_close_handler(ConnectionHandler handler) {
    m_close_handler = std::move(handler);
}
void BinanceConnection::set_fail_handler(ErrorHandler handler) {
    m_fail_handler = std::move(handler);
}
