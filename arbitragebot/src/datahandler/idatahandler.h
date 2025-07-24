#pragma once
#include "datahandler/idata.h"
#include "datahandler/matrixdata.h"

class IDataHandler {
public:
    virtual ~IDataHandler() = default;
    
    virtual std::shared_ptr<const MatrixData> getSnapshot() const = 0;

};
