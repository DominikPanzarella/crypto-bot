#pragma once

#include "parser/iparser.h"
#include "parser/jsonnode.h"



class JsonParser : public IParser {
public:

    JsonParser() = default;

    virtual JSON read(const std::string& to_parse) override;

    JSON getJson();


protected:

    JSON m_json;

};