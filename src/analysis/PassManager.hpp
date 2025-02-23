#pragma once
#include "analysis/Analysis.hpp"
#include "analysis/Pass.hpp"

namespace rhield::passes {

    template<uint32_t bitSize = 64>
    class PassRound {
      private:
        using PassPtr = std::shared_ptr<Pass<bitSize>>;

        // (order, pass)
        std::multimap<uint8_t, PassPtr> m_passes {};

      public:
        PassRound() = default;
        ~PassRound() = default;

        const std::multimap<uint8_t, PassPtr>& getPasses() const {
            return m_passes;
        }

        template<typename T>
        PassRound& addPass(uint8_t order, T&& pass) {
            auto ptr = std::make_shared<std::remove_reference_t<T>>(pass);

            m_passes.emplace(order, std::move(ptr));

            return *this;
        }
    };

    template<uint32_t bitSize = 64>
    class PassManager {
      private:
        std::vector<PassRound<bitSize>> m_rounds;

        uint64_t m_baseAddress; // find a better way to store this

      public:
        PassManager() = default;
        ~PassManager() = default;

        PassManager& addRound(PassRound<bitSize>& round) {
            m_rounds.emplace_back(round);

            return *this;
        }

        void runOnFunction(std::shared_ptr<analysis::Function<bitSize>> func) {
            m_baseAddress = func->getBaseAddr();

            TRACE("bb: '{}' has '{}' blocks", func->getName(), func->getDecoder().getBasicBlocks().size());

            for (auto& round : m_rounds) {
                // run function wide passes
                for (auto& [order, pass] : round.getPasses()) {
                    if (pass->getPassType() != ePassType::FUNCTION_PASS)
                        continue;

                    std::shared_ptr<FunctionPass<bitSize>> funcPass = std::static_pointer_cast<FunctionPass<bitSize>>(pass);

                    funcPass->run(func);
                }

                for (auto* node = func->getDecoder().getProgram().getHead(); node != nullptr; node = node->getNext()) {
                    for (auto& [order, pass] : round.getPasses()) {
                        if (pass->getPassType() != ePassType::INSTRUCTION_PASS)
                            continue;

                        std::shared_ptr<InstructionPass<bitSize>> instrPass = std::static_pointer_cast<InstructionPass<bitSize>>(pass);

                        instrPass->run(func->getDecoder().getProgram(), node, func->getDecoder().getAssembler(), m_baseAddress);
                    }
                }
            }
        }
    };
}  // namespace rhield::passes