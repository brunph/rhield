#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class RetPass: public FunctionPass<bitSize> {
      private:
      public:
        RetPass()
            : FunctionPass<bitSize>("Ret") {
        }

        void run(std::shared_ptr<analysis::Function<bitSize>> func) override {
            using namespace zasm;

            auto& decoder = func->getDecoder();
            auto& program = decoder.getProgram();
            auto& a = decoder.getAssembler();

            for (auto* node = program.getHead(); node != nullptr; node = node->getNext()) {
                if (auto* instr = node->getIf<Instruction>(); instr != nullptr) {
                    if (instr->getMnemonic() != x86::Mnemonic::Ret)
                        continue;

                    a.setCursor(node);
                    program.destroy(node);

                    a.lea(x86::rsp, x86::qword_ptr(x86::rsp, 0x8));
                    a.jmp(x86::qword_ptr(x86::rsp, -0x8));
                }
            }
        }
    };
}  // namespace rhield::passes