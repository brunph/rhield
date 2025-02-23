#pragma once

namespace rhield::util {
    std::string getHexDump(const uint8_t* buf, size_t len);
    std::string getFormattedAddress(uint64_t addr);
    std::string getDisassemblyDump(const zasm::Serializer& serializer, const zasm::MachineMode mode);

    constexpr std::array<const char*, 40> g_SectionNames = {
        "umbrella",
        "mountain",
        "butterfly",
        "elephant",
        "giraffe",
        "sunflower",
        "waterfall",
        "breeze",
        "cinnamon",
        "rainbow",
        "whisper",
        "seashell",
        "carousel",
        "firefly",
        "thunder",
        "mystery",
        "harmony",
        "journey",
        "serenity",
        "twilight",
        "crimson",
        "peacock",
        "lagoon",
        "fireworks",
        "enchanted",
        "sparkle",
        "serendipity",
        "moonlight",
        "lullaby",
        "triumph",
        "whimsical",
        "treasure",
        "captivating",
        "radiance",
        "tranquility",
        "cascade",
        "whispering",
        "wonderland",
        "euphoria"
    };

    template<typename T>
    inline T pickRandomItem(const T* arr, size_t size) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, size - 1);

        return arr[dis(gen)];
    }

    inline std::vector<std::uint8_t> readFile(const std::filesystem::path& path) {
        std::fstream file(path, std::ios::in | std::ios::binary);
        if (!file) {
            return {};
        }

        file.seekg(0, std::fstream::end);
        const auto f_size = file.tellg();
        file.seekg(0, std::fstream::beg);

        std::vector<uint8_t> buffer(static_cast<const unsigned int>(f_size));

        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

        return buffer;
    }

    inline std::expected<std::vector<std::uint8_t>, zasm::Error> encodeJmp(const zasm::MachineMode machine_mode, const uintptr_t source, const uintptr_t destination) {
        zasm::Program program(machine_mode);
        zasm::x86::Assembler assembler(program);

        assembler.jmp(zasm::Imm(destination));

        zasm::Serializer serializer;
        if (auto res = serializer.serialize(program, source); res.getCode() != zasm::ErrorCode::None) {
            return std::unexpected(res);
        }

        std::vector<std::uint8_t> result = {};
        result.resize(serializer.getCodeSize());
        std::memcpy(result.data(), serializer.getCode(), result.size());

        return result;
    }

    inline bool isReturn(const zasm::Instruction& instr) {
        return instr.getMnemonic() == zasm::x86::Mnemonic::Ret;
    }

    inline bool isJmp(const zasm::Instruction& instr) {
        using namespace zasm::x86;

        return (instr.getMnemonic() >= Mnemonic::Jb && instr.getMnemonic() <= Mnemonic::Jz);
    }

    static std::size_t estimateCodeSize(const zasm::Program& program) {
        std::size_t size = 0;

        for (auto* node = program.getHead(); node != nullptr; node = node->getNext()) {
            if (auto* nodeData = node->getIf<zasm::Data>(); nodeData != nullptr) {
                size += nodeData->getTotalSize();
            } else if (auto* nodeInstr = node->getIf<zasm::Instruction>(); nodeInstr != nullptr) {
                const auto& instrInfo = nodeInstr->getDetail(program.getMode());

                if (instrInfo.hasValue()) {
                    size += instrInfo->getLength();
                }
            } else if (auto* nodeEmbeddedLabel = node->getIf<zasm::EmbeddedLabel>(); nodeEmbeddedLabel != nullptr) {
                const auto bitSize = nodeEmbeddedLabel->getSize();
             
                if (bitSize == zasm::BitSize::_32)
                    size += 4;
                if (bitSize == zasm::BitSize::_64)
                    size += 8;
            }
        }
        return size;
    }

}  // namespace rhield::util