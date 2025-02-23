#pragma once
#include "analysis/Analysis.hpp"

namespace rhield::passes {
    enum class ePassType {
        FUNCTION_PASS,
        INSTRUCTION_PASS
    };

    template<uint32_t bitSize = 64>
    class Pass {
      private:
        std::string m_name {};
        ePassType m_passType;

      public:
        Pass(std::string name, ePassType passType)
            : m_name(name)
            , m_passType(passType) {
        }

        const std::string& getName() const {
            return m_name;
        }

        const ePassType getPassType() const {
            return m_passType;
        }
    };

    template<uint32_t bitSize = 64>
    class FunctionPass: public Pass<bitSize> {
      private:
      public:
        FunctionPass(std::string name)
            : Pass<bitSize>(name, ePassType::FUNCTION_PASS) {
        }

        virtual void run(std::shared_ptr<analysis::Function<bitSize>> func) {};
    };

    template<uint32_t bitSize = 64>
    class InstructionPass: public Pass<bitSize> {
      private:
      public:
        InstructionPass(std::string name)
            : Pass<bitSize>(name, ePassType::INSTRUCTION_PASS) {
        }

        virtual void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddress) {};
    };

}  // namespace rhield::passes