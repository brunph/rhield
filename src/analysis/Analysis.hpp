#pragma once
#include "CommonIncludes.hpp"
#include "Decoder/Decoder.hpp"

namespace rhield::analysis {
    template<uint32_t bitSize = 64>
    class Function {
      private:
        uint32_t m_rva {};
        std::string m_name {};
        Decoder m_decoder;
        size_t m_originalSize {};
        bool m_hasAnalyzed {};
        uint64_t m_baseAddress {};

      public:
        Function(const std::string& name, uint32_t rva, zasm::MachineMode machineMode = zasm::MachineMode::AMD64)
            : m_name(name)
            , m_rva(rva)
            , m_decoder(machineMode) {
            TRACE("Created analyzed function for: {} (0x{:X})", m_name, m_rva);
        }
        ~Function() = default;

        Decoder& getDecoder() {
            return m_decoder;
        }

        uint32_t getRva() const {
            return m_rva;
        }

        uint32_t getStart() {
            return m_decoder.getBasicBlocks().front().rva;
        }

        uint32_t getEnd() {
            return m_decoder.getBasicBlocks().back().rva + m_decoder.getBasicBlocks().back().size;
        }

        const std::string& getName() const {
            return m_name;
        }

        uint64_t getBaseAddr() const {
            return m_baseAddress;
        }

        void analyze(std::unique_ptr<pepp::Image<bitSize>>& img) {
            m_decoder.decodeFunction(img, m_rva);

            m_decoder.followBasicBlocks(img);

            m_hasAnalyzed = true;
            m_originalSize = m_decoder.getSize();

            m_baseAddress = img->getImageBase();

            INFO("Finished analyzing of '{}' ({} blocks)", m_name, m_decoder.getBasicBlocks().size());
        }

        void recalculateSize() {
            m_decoder.recalculateSize();
        }

        size_t getSize() {
            return m_decoder.getSize();
        }

        size_t getOgSize() {
            return m_originalSize;
        }

        std::pair<const uint8_t*, size_t> serialize(uint64_t newAddress) {
            auto res = m_decoder.serialize(newAddress);

            if (!res) {
                CRITICAL("Failed to serialize '{}' due to: {}", m_name, res.error());
                // TODO: better handling of this
            }

            INFO("Successfully serialized '{}'", m_name);

            return *res;
        }
    };

    template<uint32_t bitSize = 64>
    class FunctionAnalyzer {
      private:
        std::vector<std::shared_ptr<Function<bitSize>>> m_functions {};
        zasm::MachineMode m_machineMode {};

      public:
        FunctionAnalyzer(zasm::MachineMode machineMode = zasm::MachineMode::AMD64)
            : m_machineMode(machineMode) {
        }
        ~FunctionAnalyzer() = default;

        FunctionAnalyzer& addFunction(const std::string& name, uint32_t rva) {
            m_functions.emplace_back(std::make_shared<Function<bitSize>>(name, rva, m_machineMode));

            return *this;
        }

        const std::vector<std::shared_ptr<Function<bitSize>>>& getFunctions() {
            return m_functions;
        }

        const size_t getTotalSize() const {
            size_t result {};

            for (auto& func : m_functions) {
                result += func->getSize() + 0x10;  // 0x10 being padding
            }

            return result;
        }

        void recalculateSizes() {
            for (auto& func : m_functions) {
                func->recalculateSize();
            }
        }

        void run(std::unique_ptr<pepp::Image<bitSize>>& img) {
            INFO("Starting function analysis");
            for (auto& func : m_functions) {
                func->analyze(img);
                func->recalculateSize();
                INFO("  - '{}' [{:X}-{:X}]", func->getName(), func->getStart(), func->getEnd());
            }
            INFO("Finished analyzing '{}' functions.", m_functions.size());

            TRACE("total size: {}", getTotalSize());
        }

        void serialize(std::unique_ptr<pepp::Image<bitSize>>& img) {
            pepp::SectionHeader& rhieldSect = img->getSectionHdr(".rhield");

            size_t offset = 0x0;
            constexpr auto PAD = 0x10;

            for (auto& func : m_functions) {
                auto startAddress = img->getImageBase() + func->getRva();
                auto newAddress = img->getImageBase() + rhieldSect.getVirtualAddress() + offset;

                auto jmpData = util::encodeJmp(m_machineMode, startAddress, newAddress);

                if (!jmpData) {
                    CRITICAL("Failed to encode jump.");
                    break;
                }

                // insert the jmp to our new code
                std::memcpy(img->base() + img->getPEHdr().rvaToOffset(func->getRva()), jmpData->data(), jmpData->size());

                img->scrambleVaData(func->getRva() + jmpData->size(), func->getOgSize() - jmpData->size());

                // generate our new code
                auto [pData, pSize] = func->serialize(newAddress);

                auto hdrRva = rhieldSect.getVirtualAddress() + offset;
                auto hdrOffset = img->getPEHdr().rvaToOffset(hdrRva);

                std::memcpy(img->base() + hdrOffset, pData, pSize);

                offset += func->getSize() + PAD;
            }
        }
    };

}  // namespace rhield::analysis