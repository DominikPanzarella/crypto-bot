#pragma once

#include <functional>
#include <string>

class IConnection {
public:
    virtual ~IConnection() = default;

    using MessageHandler = std::function<void(const std::string&)>;
    using ConnectionHandler = std::function<void()>;
    using ErrorHandler = std::function<void(const std::string&)>;

    virtual void connect(const std::string& uri) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual void send(const std::string& message, std::function<void(const std::string&)> callback = nullptr) = 0;

    virtual void set_message_handler(MessageHandler handler) = 0;
    virtual void set_open_handler(ConnectionHandler handler) = 0;
    virtual void set_close_handler(ConnectionHandler handler) = 0;
    virtual void set_fail_handler(ErrorHandler handler) = 0;
};
