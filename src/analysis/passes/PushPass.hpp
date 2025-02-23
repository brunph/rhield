#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class PushPass: public InstructionPass<bitSize> {
      private:
      public:
        PushPass()
            : InstructionPass<bitSize>("Push") {
        }

        void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddress) override {
            using namespace zasm;

            Instruction* p = node->getIf<Instruction>();

            if (!p)
                return;

            if (p->getMnemonic() != x86::Mnemonic::Push)
                return;

            auto* op = p->getOperandIf<Reg>(0);
            if (!op)
                return;

            auto opId = op->getId();

            a.setCursor(node);
            program.destroy(node);

            a.lea(x86::rsp, x86::qword_ptr(x86::rsp, -8));
            a.mov(x86::qword_ptr(x86::rsp), zasm::x86::Gp { opId });
        }
    };
}  // namespace rhield::passes