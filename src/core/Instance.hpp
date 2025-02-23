#pragma once
#include "CommonIncludes.hpp"
#include "util/FileParser.hpp"
#include "util/MapParser.hpp"
#include "analysis/Analysis.hpp"
#include "analysis/PassManager.hpp"
#include "analysis/passes/NopPass.hpp"
#include "analysis/passes/RetPass.hpp"
#include "analysis/passes/PushPass.hpp"
#include "analysis/passes/PopPass.hpp"
#include "analysis/passes/JunkPass.hpp"
#include "analysis/passes/LeaPass.hpp"
#include "analysis/passes/CallPass.hpp"
#include "analysis/passes/DebugCheckPass.hpp"
#include "analysis/passes/SubstitutionPass.hpp"

namespace rhield {
    template<uint32_t bitSize = 64>
    class Instance {
      private:
        std::unique_ptr<pepp::Image<64>> m_img;  // TODO: change back to using bitsize, only doing it for intellisense rn
        std::filesystem::path m_path {};

        std::unique_ptr<parser::Parser> m_parser {};
        parser::eParserType m_curParserType {};

        analysis::FunctionAnalyzer<bitSize> m_functionAnalyzer;

        passes::PassManager<bitSize> m_passManager {};

        std::vector<parser::ParsedFunction> m_targetFunctions {};

        bool m_randomizeSections {};

      public:
        explicit Instance(const std::filesystem::path& path)
            : m_path(path) {
            m_img = std::make_unique<pepp::Image<bitSize>>(path.string());
        }

        Instance& setMapParser(const std::filesystem::path& mapPath) {
            m_parser = std::make_unique<parser::MapParser>(mapPath);
            m_curParserType = parser::eParserType::MAP_PARSER;

            return *this;
        }

        Instance& addFunctionByName(const std::string& name) {
            auto pFunc = m_parser->findFunctionByName(name);

            if (!pFunc) {
                CRITICAL("Failed to find function by name: {}", name);
                return *this;
            }

            m_targetFunctions.emplace_back(*pFunc);

            return *this;
        }

        Instance& addFunctionByRva(uint32_t rva) {
            auto pFunc = m_parser->findFunctionByRva(rva);

            if (!pFunc) {
                CRITICAL("Failed to find function by rva: 0x{:X}", rva);
                return *this;
            }

            m_targetFunctions.emplace_back(*pFunc);

            return *this;
        }

        Instance& setRandomizeSections(bool val) {
            m_randomizeSections = val;

            return *this;
        }

        bool init() {
            INFO("Rhield initialized.");
            INFO("File path: {}", std::filesystem::absolute(m_path).string());

            if (!m_parser) {
                TRACE("No parser was given, trying with default map parser.");

                std::filesystem::path mapFile = m_path;
                mapFile = mapFile.replace_extension(".map");

                if (!std::filesystem::exists(mapFile)) {
                    CRITICAL("No default map file exists, please make sure the parser is correctly set.");
                    return false;
                }

                m_parser = std::make_unique<parser::MapParser>(mapFile);
                m_curParserType = parser::eParserType::MAP_PARSER;
            }

            if (!m_parser->doParse()) {
                CRITICAL("Failed to parse function list!");
                return false;
            }

            INFO("Parsed function count: {}", m_parser->getParsedFunctions().size());

            random::impl::seed();

            passes::PassRound pOne;

            //pOne.addPass(0, passes::RetPass<bitSize>());
            //pOne.addPass(0, passes::PushPass<bitSize>());
            //pOne.addPass(0, passes::PopPass<bitSize>());
            //
            //pOne.addPass(1, passes::JunkPass<bitSize>()
            //    .setLowerRounds(40)
            //    .setUpperRounds(60));

            //pOne.addPass(1, passes::CallPass<bitSize>());
            pOne.addPass(0, passes::DebugCheckPass<bitSize>());
            //pOne.addPass(0, passes::SubstitutionPass<bitSize>());

            return true;
        }

        bool run() {
            if (m_targetFunctions.empty()) {
                CRITICAL("No functions were targeted for protection.");
                return false;
            }

            INFO("Targeted function count: {}", m_targetFunctions.size());
            TRACE("First function: '{}' (0x{:X})", m_targetFunctions[0].name, m_targetFunctions[0].rva);

            for (auto& targetFunc : m_targetFunctions) {
                m_functionAnalyzer.addFunction(targetFunc.name, targetFunc.rva);
            }

            m_functionAnalyzer.run(m_img);

            for (auto& func : m_functionAnalyzer.getFunctions()) {
                m_passManager.runOnFunction(func);
            }

            m_functionAnalyzer.recalculateSizes();

            pepp::SectionHeader rhieldSect;
            if (!m_img->appendSection(".rhield", m_functionAnalyzer.getTotalSize(), pepp::SectionCharacteristics::SCN_MEM_READ | pepp::SectionCharacteristics::SCN_MEM_EXECUTE | pepp::SectionCharacteristics::SCN_CNT_CODE, &rhieldSect))  // TODO: add actual program size here
            {
                CRITICAL("Failed to add Rhield section!");
                return false;
            }

            m_functionAnalyzer.serialize(m_img);

            return true;
        }

        bool save(const std::filesystem::path& newPath) {
            // verify that we have done something
            auto& sect = m_img->getSectionHdr(".rhield");

            if (sect.getName() == ".dummy") {
                CRITICAL("No Rhield section was created.");
                return false;
            }

            const auto absPath = std::filesystem::absolute(newPath).string();

            if (m_randomizeSections) {
                uint16_t totalSections = m_img->getNumberOfSections();

                std::set<std::string> usedNames;

                for (uint16_t curSecIdx = 0; curSecIdx < totalSections; curSecIdx++) {
                    pepp::SectionHeader& sect = m_img->getSectionHdr(curSecIdx);

                    std::string newName = util::pickRandomItem<const char*>(util::g_SectionNames.data(), util::g_SectionNames.size() - 1);

                    while (usedNames.contains(newName))
                        newName = util::pickRandomItem<const char*>(util::g_SectionNames.data(), util::g_SectionNames.size() - 1);

                    usedNames.insert(newName);

                    newName.resize(7);

                    sect.setName(newName);
                }
            }

            INFO("Saving to: {}", absPath);
            m_img->writeToFile(absPath);

            return true;
        }
    };
}  // namespace rhield