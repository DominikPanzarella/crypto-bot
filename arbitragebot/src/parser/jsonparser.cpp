#include "parser/jsonparser.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

JSON JsonParser::read(const std::string& to_parse) {
    try {
        // Parsing della stringa in un oggetto nlohmann::json
        nlohmann::json parsed = nlohmann::json::parse(to_parse);

        // Crea il nodo di implementazione
        std::shared_ptr<JSONNodeImpl> impl = std::make_shared<JSONNodeImpl>(std::move(parsed));

        // Costruisci il JSON wrapper
        m_json = JSON(impl);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Errore nel parsing JSON: ") + e.what());
    }

    return m_json;
}

JSON JsonParser::getJson() {
    return m_json;
}