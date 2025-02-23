#pragma once
#include "CommonIncludes.hpp"
#include "util/FileParser.hpp"

#include <regex>
#include <utility>

namespace rhield::parser {
    class MapParser: public Parser {
      private:
        std::filesystem::path m_path;

        static std::uint64_t parseHexString(const std::string& value) {
            return std::stoull(value, nullptr, 16);
        }

      public:
        explicit MapParser(std::filesystem::path path)
            : m_path(std::move(path)) {
        }
        bool doParse() override {
            auto fileData = util::readFile(m_path);

            if (fileData.empty()) {
                CRITICAL("Failed to read map file data.");
                return false;
            }

            std::stringstream strStream;
            strStream.str(std::string((char*)fileData.data(), fileData.size()));

            const std::regex mapRegex(R"(([0-9a-fA-F]+):([0-9a-fA-F]+)\s+([^\s]+))");
            std::smatch matches;

            bool parsingSymbols = false;

            for (std::string line; std::getline(strStream, line);) {
                if (line.empty())
                    continue;

                if (!parsingSymbols) {
                    parsingSymbols = line.find("ddress") != std::string::npos &&  //
                        line.find("ublics by Value") != std::string::npos;
                    continue;
                }

                if (line.find("ntry point at") != std::string::npos ||  //
                    line.find("tatic symbols") != std::string::npos) {
                    break;
                }

                if (!std::regex_search(line, matches, mapRegex)) {
                    continue;
                }

                // TODO: parse the section start address automatically.
                // .text might not always start at 0x1000

                auto sectionIdx = parseHexString(matches[1]);

                if (sectionIdx != 1)  // isnt .text
                    continue;

                constexpr auto SECTION_BASE = 0x1000;  // .text usually starts at this

                auto sectionOffset = parseHexString(matches[2]);

                getParsedFunctions().emplace_back(matches[3], SECTION_BASE + sectionOffset);
            }

            return true;
        }
    };
}  // namespace rhield::parser