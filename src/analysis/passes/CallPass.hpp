#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class CallPass: public InstructionPass<bitSize> {
      private:
      public:
        CallPass()
            : InstructionPass<bitSize>("Call") {
        }

        void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddress) override {
            using namespace zasm;

            Instruction* p = node->getIf<Instruction>();

            if (p && p->getMnemonic() == x86::Mnemonic::Call) {
                if (auto* lbl = p->getOperandIf<Imm>(0); lbl != nullptr) {
                    auto callKey = random::number<uintptr_t>();

                    auto newImm = (lbl->value<uint64_t>() - baseAddress);

                    newImm ^= callKey;
                    callKey = _rotr64(callKey, 16);

                    a.setCursor(node->getPrev());
                    program.destroy(node);

                    a.mov(x86::r14, newImm);
                    a.mov(x86::r15, callKey);
                    a.rol(x86::r15, 0x10);
                    a.xor_(x86::r14, x86::r15);
                    a.mov(x86::r15, x86::qword_ptr(x86::gs, 0x60));
                    a.mov(x86::r15, x86::qword_ptr(x86::r15, 0x10));
                    a.add(x86::r15, x86::r14);

                    a.call(x86::r15);
                }
            }
        }
    };
}  // namespace rhield::passes