#pragma once

#include <string>
#include "parser/jsonnode.h"

class IParser {
public: 
    virtual JSON read(const std::string& to_parse) =0 ;
};