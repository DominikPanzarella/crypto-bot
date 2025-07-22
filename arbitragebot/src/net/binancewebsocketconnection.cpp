#include "net/binancewebsocketconnection.h"
#include <websocketpp/common/thread.hpp>
#include <iostream>

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

BinanceWebSocketConnection::BinanceWebSocketConnection() : m_is_connected(false), m_running(false) {
    m_client.clear_access_channels(websocketpp::log::alevel::all);
    m_client.clear_error_channels(websocketpp::log::elevel::all);

    m_client.init_asio();

    m_client.set_tls_init_handler(bind(&BinanceWebSocketConnection::on_tls_init, this));
    m_client.set_message_handler(bind(&BinanceWebSocketConnection::handle_message, this, _1, _2));
    m_client.set_open_handler(bind(&BinanceWebSocketConnection::handle_open, this, _1));
    m_client.set_close_handler(bind(&BinanceWebSocketConnection::handle_close, this, _1));
    m_client.set_fail_handler(bind(&BinanceWebSocketConnection::handle_fail, this, _1));
}

BinanceWebSocketConnection::~BinanceWebSocketConnection() {
    disconnect();
}

BinanceWebSocketConnection::ContextPtr BinanceWebSocketConnection::on_tls_init() {
    auto ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12_client);
    try {
        ctx->set_options(asio::ssl::context::default_workarounds |
                         asio::ssl::context::no_sslv2 |
                         asio::ssl::context::no_sslv3 |
                         asio::ssl::context::single_dh_use);

        ctx->set_default_verify_paths();

        const char* cert_paths[] = {
            "/etc/ssl/certs/ca-certificates.crt", // Debian/Ubuntu
            "/etc/pki/tls/certs/ca-bundle.crt",   // RHEL/CentOS
            "/usr/local/etc/openssl/cert.pem",    // macOS
            nullptr
        };

        for (int i = 0; cert_paths[i]; ++i) {
            try {
                ctx->load_verify_file(cert_paths[i]);
                break;
            } catch (...) {
                continue;
            }
        }
    } catch (std::exception& e) {
        if (m_fail_handler) {
            m_fail_handler(std::string("TLS Error: ") + e.what());
        }
    }
    return ctx;
}

void BinanceWebSocketConnection::connect(const std::string& uri) {
    if (is_connected()) {
        if (m_fail_handler) m_fail_handler("Already connected");
        return;
    }

    websocketpp::lib::error_code ec;
    auto con = m_client.get_connection(uri, ec);
    if (ec) {
        if (m_fail_handler) m_fail_handler(ec.message());
        return;
    }

    m_connection = con->get_handle();
    m_client.connect(con);

    m_running = true;

    m_poll_thread = std::thread([this]() {
        while (m_running) {
            try {
                m_client.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } catch (...) {}
        }
    });

    m_send_thread = std::thread([this]() {
        while (m_running) {
            std::unique_lock<std::mutex> lock(m_send_mutex);
            m_send_cv.wait(lock, [this] { return !m_send_queue.empty() || !m_running; });

            while (!m_send_queue.empty()) {
                auto [msg, callback] = m_send_queue.front();
                m_send_queue.pop();
                lock.unlock();

                websocketpp::lib::error_code ec;
                m_client.send(m_connection, msg, websocketpp::frame::opcode::text, ec);

                if (ec) {
                    const std::string err = ec.message();
                    if (callback) callback(err);
                    if (m_fail_handler) m_fail_handler(err);
                } else if (callback) {
                    callback("");
                }

                lock.lock();
            }
        }
    });
}

void BinanceWebSocketConnection::disconnect() {
    if (!is_connected()) return;

    m_running = false;
    m_send_cv.notify_all();

    if (m_poll_thread.joinable()) m_poll_thread.join();
    if (m_send_thread.joinable()) m_send_thread.join();

    websocketpp::lib::error_code ec;
    m_client.close(m_connection, websocketpp::close::status::normal, "Disconnecting", ec);
    if (ec && m_fail_handler) {
        m_fail_handler(ec.message());
    }
}

bool BinanceWebSocketConnection::is_connected() const {
    return m_is_connected.load();
}

void BinanceWebSocketConnection::send(const std::string& message,
                                      std::function<void(const std::string&)> callback) {
    if (!is_connected()) {
        const std::string err = "Not connected";
        if (callback) callback(err);
        if (m_fail_handler) m_fail_handler(err);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_send_mutex);
        m_send_queue.emplace(message, callback);
    }
    m_send_cv.notify_one();
}

void BinanceWebSocketConnection::set_message_handler(MessageHandler handler) {
    m_message_handler = handler;
}

void BinanceWebSocketConnection::set_open_handler(ConnectionHandler handler) {
    m_open_handler = handler;
}

void BinanceWebSocketConnection::set_close_handler(ConnectionHandler handler) {
    m_close_handler = handler;
}

void BinanceWebSocketConnection::set_fail_handler(ErrorHandler handler) {
    m_fail_handler = handler;
}

void BinanceWebSocketConnection::set_debug(bool enable) {
    if (enable) {
        m_client.set_access_channels(websocketpp::log::alevel::all);
        m_client.set_error_channels(websocketpp::log::elevel::all);
    } else {
        m_client.clear_access_channels(websocketpp::log::alevel::all);
        m_client.clear_error_channels(websocketpp::log::elevel::all);
    }
}

void BinanceWebSocketConnection::handle_message(websocketpp::connection_hdl, Client::message_ptr msg) {
    if (m_message_handler) {
        m_message_handler(msg->get_payload());
    }
}

void BinanceWebSocketConnection::handle_open(websocketpp::connection_hdl hdl) {
    m_is_connected.store(true);
    if (m_open_handler) m_open_handler();
}

void BinanceWebSocketConnection::handle_close(websocketpp::connection_hdl hdl) {
    m_is_connected.store(false);
    if (m_close_handler) m_close_handler();
}

void BinanceWebSocketConnection::handle_fail(websocketpp::connection_hdl hdl) {
    m_is_connected.store(false);
    if (m_fail_handler) {
        m_fail_handler("Connection to Binance WebSocket failed");
    }
}
