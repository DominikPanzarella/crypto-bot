#include "net/binancewebsocketconnection.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

std::atomic<bool> running{true};

// Signal handler per terminare con Ctrl+C
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupted. Shutting down..." << std::endl;
        running = false;
    }
}

int main() {
    std::signal(SIGINT, signal_handler); // Registra handler per Ctrl+C

    try {
        BinanceWebSocketConnection binanceWS;

        // 1. Abilita debug avanzato (opzionale)
        binanceWS.set_debug(true);

        // 2. Configura handlers
        binanceWS.set_open_handler([&]() {
            std::cout << "CONNECTED! Sending subscription..." << std::endl;

            // 3. Invia sottoscrizione a un stream
            const std::string sub_msg = R"({"method":"SUBSCRIBE","params":["bnbusdt@ticker"],"id":1})";
            binanceWS.send(sub_msg, [](const std::string& err) {
                if (!err.empty()) {
                    std::cerr << "SEND ERROR: " << err << std::endl;
                }
            });
        });

        binanceWS.set_message_handler([](const std::string& msg) {
            std::cout << "RAW MESSAGE: " << msg << std::endl;
        });

        binanceWS.set_fail_handler([](const std::string& error) {
            std::cerr << "ERROR: " << error << std::endl;
        });

        binanceWS.set_close_handler([]() {
            std::cout << "Connection closed." << std::endl;
        });

        // 4. Connessione all'endpoint WebSocket di Binance
        std::cout << "Connecting..." << std::endl;
        binanceWS.connect("wss://stream.binance.com:9443/ws");

        // 5. Loop principale per processare eventi
        while (running) {
            binanceWS.poll(); // Processa eventi (messaggi, connessioni, errori, ecc.)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // 6. Disconnessione pulita
        binanceWS.disconnect();
        std::cout << "Shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
