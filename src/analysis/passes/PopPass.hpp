#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class PopPass: public InstructionPass<bitSize> {
      private:
      public:
        PopPass()
            : InstructionPass<bitSize>("Pop") {
        }

        void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddr) override {
            using namespace zasm;

            Instruction* p = node->getIf<Instruction>();

            if (!p)
                return;

            if (p->getMnemonic() != x86::Mnemonic::Pop)
                return;

            auto* op = p->getOperandIf<Reg>(0);
            if (!op)
                return;
            
            auto opId = op->getId();

            a.setCursor(node);
            program.destroy(node);

            a.mov(zasm::x86::Gp { opId }, x86::qword_ptr(x86::rsp));
            a.lea(x86::rsp, x86::qword_ptr(x86::rsp, 8));
        }
    };
}  // namespace rhield::passes