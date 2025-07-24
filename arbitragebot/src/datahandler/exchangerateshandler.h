#pragma once

#include "datahandler/idatahandler.h"
#include "observer/iobserver.h"
#include "datahandler/matrixdata.h"
#include "datahandler/idata.h"
#include <atomic>
#include <memory>
#include <mutex>

class ExchangeRatesHandler : public IDataHandler, public IObserver {
public:
    ExchangeRatesHandler(size_t size);

    virtual std::shared_ptr<const MatrixData> getSnapshot() const override;

    void onData(std::shared_ptr<IData> data) override;

private:
    std::shared_ptr<MatrixData> active_buffer_;
    
    std::shared_ptr<MatrixData> standby_buffer_;

    mutable std::mutex buffer_mutex_;

    void updateRates(MatrixData& matrix, std::shared_ptr<IData> data);
};
