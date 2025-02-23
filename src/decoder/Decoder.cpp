#include "CommonIncludes.hpp"
#include "decoder/Decoder.hpp"

namespace rhield {
    Decoder::Decoder(const zasm::MachineMode& mode)
        : m_program(mode)
        , m_decoder(mode)
        , m_assembler(m_program) {
    }

    void Decoder::recalculateSize() {
        m_programSize = util::estimateCodeSize(m_program);
    }

    void Decoder::decodeFunction(std::unique_ptr<pepp::Image64>& img, uintptr_t rva) {
        if (!m_baseAddr)
            m_baseAddr = img->getImageBase();

        auto& func = m_basicBlocks.emplace_back();
        func.rva = rva;
        func.analyzed = true;

        decodeData(func, img->base() + img->getPEHdr().rvaToOffset(rva), 0x1000000, img->getImageBase() + rva, true);
    }

    void Decoder::followBasicBlocks(std::unique_ptr<pepp::Image64>& img) {
        for (auto& basicBlock : m_basicBlocks) {
            if (basicBlock.analyzed)
                continue;

            decodeData(basicBlock, img->base() + img->getPEHdr().rvaToOffset(basicBlock.rva), 0x1000000, img->getImageBase() + basicBlock.rva, true);
            basicBlock.analyzed = true;
        }

        INFO("Discovered '{}' new basic blocks.", m_basicBlocks.size());
    }

    void Decoder::decodeData(BasicBlock& bb, const std::uint8_t* data, const std::size_t size, const uint64_t origAddress, bool runUntilReturn) {
        auto blockLabel = m_assembler.createLabel();
        m_blockLabels.emplace(origAddress, blockLabel);
        auto blockLabelNode = m_assembler.bind(blockLabel);

        std::size_t decoded = 0;
        while (decoded < size) {
            const auto decodedResult = m_decoder.decode(data + decoded, size - decoded, origAddress + decoded);
            auto currentRva = ((origAddress + decoded) - m_baseAddr);

            if (!decodedResult) {
                throw std::runtime_error("Unable to decode data");
            }

            if (const auto res = m_assembler.emit(decodedResult->getInstruction()); res.getCode() != zasm::ErrorCode::None) {
                throw std::runtime_error("Unable to encode decoded data");
            }

            if (util::isJmp(decodedResult->getInstruction())) {
                auto instr = decodedResult->getInstruction();

                if (auto* opImm = instr.getOperandIf<zasm::Imm>(0); opImm != nullptr) {
                    auto& bblock = m_basicBlocks.emplace_back();
                    bblock.analyzed = false;
                    bblock.rva = opImm->value<uint64_t>() - m_baseAddr;
                    m_jmpLocs.insert(opImm->value<uint64_t>());
                    TRACE("bb: {:X}", bblock.rva);
                }

                if (instr.getMnemonic() == zasm::x86::Mnemonic::Jmp)
                    break;
            }

            if (m_jmpLocs.contains(origAddress + decoded) && currentRva != bb.rva) {
                TRACE("interlapping blocks: ({:X}) {:X} ({:X})", bb.rva, origAddress + decoded, currentRva);
                break;
            }
            if (runUntilReturn && util::isReturn(decodedResult->getInstruction()))
                break;

            decoded += decodedResult->getLength();
        }
        bb.size += decoded;

        m_programSize += decoded;
    }

    std::expected<std::pair<const uint8_t*, size_t>, std::string> Decoder::serialize(uint64_t address) {
        for (auto* node = m_program.getHead(); node != nullptr; node = node->getNext()) {
            if (auto* instr = node->getIf<zasm::Instruction>(); instr != nullptr) {
                if (auto* opImm = instr->getOperandIf<zasm::Imm>(0); opImm != nullptr) {
                    const auto addr = opImm->value<std::uint64_t>();
                    if (auto it = m_blockLabels.find(addr); it != m_blockLabels.end()) {
                        instr->setOperand(0, it->second);
                    }
                }
            }
        }

        TRACE("serialize address: {:X}", address);
        auto res = m_serializer.serialize(m_program, address);

        if (res.getCode() != zasm::ErrorCode::None) {
            return std::unexpected(std::format("Serialization error: {}", res.getErrorMessage()));
        }

        return std::make_pair(m_serializer.getCode(), m_serializer.getCodeSize());
    }
}  // namespace rhield