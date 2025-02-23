#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class NopPass: public FunctionPass<bitSize> {
      private:
      public:
        NopPass()
            : FunctionPass<bitSize>("Nop") {
        }

        void run(std::shared_ptr<analysis::Function<bitSize>> func) override {
            auto& decoder = func->getDecoder();
            auto& program = decoder.getProgram();
            auto& a = decoder.getAssembler();

            for (auto* node = program.getHead(); node != nullptr; node = node->getNext()) {
                if (auto* instr = node->getIf<zasm::Instruction>(); instr != nullptr) {
                    a.setCursor(node->getPrev());
                    a.nop();
                }
            }
        }
    };
}  // namespace rhield::passes