#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class SubstitutionPass: public InstructionPass<bitSize> {
      private:
      public:
        SubstitutionPass()
            : InstructionPass<bitSize>("Substitution") {
        }

        void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddress) override {
            using namespace zasm;

            Instruction* p = node->getIf<Instruction>();

            if (p && p->getMnemonic() == x86::Mnemonic::Mov) {
                if (auto* reg = p->getOperandIf<Reg>(0); reg != nullptr) {
                    if (auto* mem = p->getOperandIf<Mem>(1); mem != nullptr) {
                        auto disp = mem->getDisplacement();
                        auto bSize = mem->getBitSize();
                        auto regId = reg->getId();

                        if (disp < baseAddress || bSize != BitSize::_64)
                            return;

                        auto callKey = random::number<uintptr_t>();

                        const auto rva = (disp - baseAddress);

                        auto newDisp = rva ^ callKey;
                        callKey = _rotr64(callKey, 16);

                        a.setCursor(node->getPrev());
                        program.destroy(node);

                        a.push(x86::r15);
                        a.push(x86::r14);

                        a.mov(x86::r14, newDisp);
                        a.mov(x86::r15, callKey);
                        a.rol(x86::r15, 0x10);
                        a.xor_(x86::r14, x86::r15);
                        a.mov(x86::r15, x86::qword_ptr(x86::gs, 0x60));
                        a.mov(x86::r15, x86::qword_ptr(x86::r15, 0x10));
                        a.add(x86::r15, x86::r14);

                        a.mov(x86::Gp { regId }, x86::qword_ptr(x86::r15));

                        a.pop(x86::r14);
                        a.pop(x86::r15);
                    }
                }
            }
        }
    };
}  // namespace rhield::passes