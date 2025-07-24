#include "net/binancewebsocketconnection.h"
#include "parser/jsonparser.h"

#include "datahandler/exchangerateshandler.h"

#include "observer/websocketsubject.h"

#include "datahandler/tickerdata.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <memory>

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupted. Shutting down..." << std::endl;
        running = false;
    }
}

void stampa_snapshot(const std::unordered_map<std::string, std::unordered_map<std::string, double>>& snapshot) {
    std::cout << "\n=== MATRICE TASSI DI CAMBIO ===\n";
    for (const auto& from_pair : snapshot) {
        const auto& from = from_pair.first;
        for (const auto& to_pair : from_pair.second) {
            const auto& to = to_pair.first;
            double rate = to_pair.second;
            std::cout << from << " → " << to << " : " << rate << std::endl;
        }
    }
    std::cout << "===============================\n";
}

std::unordered_map<std::string, std::unordered_map<std::string, double>> convertToMap(const MatrixData& matrix) {
    std::unordered_map<std::string, std::unordered_map<std::string, double>> result;
    for (const auto& [from_currency, i] : matrix.currency_to_index) {
        for (const auto& [to_currency, j] : matrix.currency_to_index) {
            result[from_currency][to_currency] = matrix.rates[i][j];
        }
    }
    return result;
}


int main() {
    std::signal(SIGINT, signal_handler);

    try {
        auto subject = std::make_shared<WebSocketSubject>();
        auto matrixHandler = std::make_shared<ExchangeRatesHandler>(1000); // adatta dimensione
        subject->subscribe(matrixHandler);

        BinanceConnection binanceWS;
        JsonParser parser;

        binanceWS.set_open_handler([&]() {
            std::cout << "CONNECTED! Listening to all tickers..." << std::endl;
        });

        binanceWS.set_message_handler([&parser, subject](const std::string& msg) {
            try {
                JSON json = parser.read(msg);

                if (json.is_array()) {
                    //std::cout << msg << std::endl;
                    for (int i = 0; i < static_cast<int>(json.impl_->data.size()); ++i) {
                        auto item = json[i];

                        std::string symbol   = item->impl_->data.value("s", "");
                        double lastPrice     = std::stod(item->impl_->data.value("c", "0"));
                        double bestBid       = std::stod(item->impl_->data.value("b", "0"));
                        double bestAsk       = std::stod(item->impl_->data.value("a", "0"));
                        double openPrice     = std::stod(item->impl_->data.value("o", "0"));
                        double volume        = std::stod(item->impl_->data.value("v", "0"));

                        auto data = std::make_shared<TickerData>(
                            symbol, lastPrice, bestBid, bestAsk, openPrice, volume
                        );

                        subject->notify(data);  // Notifica tutti gli observer (es. ExchangeRatesHandler)
                    }
                } else {
                    std::cerr << "Warning: Expected JSON array of tickers." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        });

        binanceWS.set_fail_handler([](const std::string& error) {
            std::cerr << "ERROR: " << error << std::endl;
        });

        binanceWS.set_close_handler([]() {
            std::cout << "Connection closed." << std::endl;
        });

        std::cout << "Connecting..." << std::endl;
        binanceWS.connect("wss://stream.binance.com:9443/ws/!ticker@arr");

        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            static int counter = 0;
            if (++counter % 50 == 0) { // stampa ogni 5 secondi circa (100ms * 50)
                auto snapshot = matrixHandler->getSnapshot();
                stampa_snapshot(convertToMap(*snapshot));
            }
            
        }

        binanceWS.disconnect();
        std::cout << "Shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
