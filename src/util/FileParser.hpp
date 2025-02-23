#pragma once
#include "CommonIncludes.hpp"

namespace rhield::parser {
    struct ParsedFunction {
        ParsedFunction(std::string name, size_t rva)
            : name(name)
            , rva(rva) {}

        std::string name {};
        size_t rva {};
    };

    enum class eParserType {
        NONE = -1,
        MAP_PARSER,
        JSON_PARSER,  // TODO: implement this
        TOML_PARSER,  // TODO: and this
    };

    class Parser {
      private:
        std::vector<ParsedFunction> m_parsedFunctions;

      public:
        Parser() = default;
        ~Parser() = default;

        std::vector<ParsedFunction>& getParsedFunctions() {
            return m_parsedFunctions;
        }

        std::optional<ParsedFunction> findFunctionByName(const std::string& funcName) {
            auto it = std::ranges::find_if(m_parsedFunctions, [&](const auto& func) {
                return func.name == funcName;
            });

            if (it != m_parsedFunctions.end())
                return *it;

            return std::nullopt;
        }

        
        std::optional<ParsedFunction> findFunctionByRva(uint32_t rva) {
            auto it = std::ranges::find_if(m_parsedFunctions, [&](const auto& func) {
                return func.rva == rva;
            });

            if (it != m_parsedFunctions.end())
                return *it;

            return std::nullopt;
        }

        virtual bool doParse() = 0;
    };
}  // namespace rhield::parser