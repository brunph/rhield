#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class DebugCheckPass: public FunctionPass<bitSize> {
      private:
      public:
        DebugCheckPass()
            : FunctionPass<bitSize>("DbgCheck") {
        }

        void run(std::shared_ptr<analysis::Function<bitSize>> func) override {
            using namespace zasm;

            auto& decoder = func->getDecoder();
            auto& program = decoder.getProgram();
            auto& a = decoder.getAssembler();

            Node* insertLocation = nullptr;

            for (auto* node = program.getHead(); node != nullptr; node = node->getNext()) {
                auto chance = random::number<size_t>((size_t)0, program.size() / 2);

                if (chance == 5) {
                    insertLocation = node;
                    break;
                }
            }

            a.setCursor(program.getTail());

            auto funcLabel = a.createLabel();

            a.bind(funcLabel);
            a.sub(x86::rsp, 48);
            a.mov(x86::rax, x86::qword_ptr(x86::gs, 0x60));
            a.movzx(x86::rax, x86::byte_ptr(x86::rax, 2));
            a.add(x86::rsp, 48);
            a.ret();

            a.setCursor(insertLocation);

            auto cond = a.createLabel();
            a.push(x86::rax);
            a.call(funcLabel);
            a.cmp(x86::rax, 0x0);
            a.jz(cond);
            a.mov(x86::dword_ptr(0x0), 0xDEADBEEF);

            a.bind(cond);
            a.pop(x86::rax);
        }
    };
}  // namespace rhield::passes