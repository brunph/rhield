#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class LeaPass: public InstructionPass<bitSize> {
      private:
      public:
        LeaPass()
            : InstructionPass<bitSize>("Lea") {
        }

        void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddress) override {
            using namespace zasm;

            Instruction* p = node->getIf<Instruction>();

            if (p && p->getMnemonic() == x86::Mnemonic::Lea) {
                auto addVal = random::number<uint32_t>(INT32_MAX / 2, INT32_MAX);

                if (auto* reg = p->getOperandIf<Reg>(0); reg != nullptr) {
                    if (auto* lbl = p->getOperandIf<Mem>(1); lbl != nullptr) {
                        if (x86::Gp { reg->getId() } != x86::rbp)
                            return;
                        
                        lbl->setDisplacement(addVal);

                        a.setCursor(node);
                        a.pushf();
                        a.push(x86::r15);
                        a.xor_(x86::r15, x86::r15);
                        a.mov(x86::r15, addVal);
                        a.neg(x86::r15);
                        a.add(x86::rbp, x86::r15);
                        a.pop(x86::r15);
                        a.popf();
                    }
                }
            }
        }
    };
}  // namespace rhield::passes