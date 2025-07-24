#include "datahandler/exchangerateshandler.h"
#include <iostream>

ExchangeRatesHandler::ExchangeRatesHandler(size_t size) : active_buffer_(std::make_shared<MatrixData>(size)),
  standby_buffer_(std::make_shared<MatrixData>(size))
{}

void ExchangeRatesHandler::updateRates(MatrixData& matrix, std::shared_ptr<IData> data) {
    const std::string& sym = data->getSymbol();
    double price = data->getLastPrice();

    // Estrarre le valute dalla coppia es: BTCUSDT -> BTC, USDT
    std::string from = sym.substr(0, 3);
    std::string to = sym.substr(3);

    int i = matrix.currency_to_index[from];
    int j = matrix.currency_to_index[to];
    matrix.rates[i][j] = price;
    if (price != 0.0)
        matrix.rates[j][i] = 1.0 / price;
}

void ExchangeRatesHandler::onData(std::shared_ptr<IData> data){
    updateRates(*standby_buffer_, data);

    // Atomic swap
    std::lock_guard lock(buffer_mutex_);
    std::swap(active_buffer_, standby_buffer_);
}


std::shared_ptr<const MatrixData> ExchangeRatesHandler::getSnapshot() const {
    std::lock_guard lock(buffer_mutex_);
    return active_buffer_;
}
