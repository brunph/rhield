#pragma once
#include "CommonIncludes.hpp"

namespace rhield {
    struct BasicBlock {
        uint32_t rva {};
        size_t size {};
        bool analyzed = false;

        bool operator==(BasicBlock& first) {
            return rva == first.rva;
        }
    };

    class Decoder {
      private:
        zasm::Program m_program;
        zasm::Decoder m_decoder;
        zasm::x86::Assembler m_assembler;

        std::map<uint64_t, zasm::Label> m_blockLabels;
        std::vector<BasicBlock> m_basicBlocks;
        std::set<uint64_t> m_jmpLocs;

        zasm::Serializer m_serializer;

        uint64_t m_baseAddr {};
        size_t m_programSize {};

      public:
        Decoder(const zasm::MachineMode& mode = zasm::MachineMode::AMD64);
        ~Decoder() = default;

        void recalculateSize();

        zasm::Program& getProgram() {
            return m_program;
        }

        zasm::x86::Assembler& getAssembler() {
            return m_assembler;
        }

        std::vector<BasicBlock>& getBasicBlocks() {
            return m_basicBlocks;
        }

        const size_t getSize() const {
            return m_programSize;
        }

        void decodeFunction(std::unique_ptr<pepp::Image64>& img, uintptr_t rva);

        void followBasicBlocks(std::unique_ptr<pepp::Image64>& img);

        // if 'runUntilReturn' is true, then size needs to be set really high so it can find the return
        void decodeData(BasicBlock& bb, const std::uint8_t* data, const std::size_t size, const uint64_t origAddress, bool runUntilReturn = false);

        std::expected<std::pair<const uint8_t*, size_t>, std::string> serialize(uint64_t address);
    };
}  // namespace rhield