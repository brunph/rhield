#pragma once
#include "analysis/Pass.hpp"

namespace rhield::passes {
    template<uint32_t bitSize>
    class JunkPass: public InstructionPass<bitSize> {
      private:
        uint32_t m_lowerRounds = 10;
        uint32_t m_upperRounds = 20;

      public:
        JunkPass()
            : InstructionPass<bitSize>("Junk") {
        }

        JunkPass& setLowerRounds(uint32_t val) {
            m_lowerRounds = val;

            return *this;
        }

        JunkPass& setUpperRounds(uint32_t val) {
            m_upperRounds = val;

            return *this;
        }

        void run(zasm::Program& program, zasm::Node* node, zasm::x86::Assembler& a, uint64_t baseAddress) override {
            using namespace zasm;

            Instruction* p = node->getIf<Instruction>();

            if (!p)
                return;

            if (node == program.getHead() || node == program.getTail())
                return;

            a.setCursor(node->getPrev());

            auto rounds = random::number(m_lowerRounds, m_upperRounds);

            for (auto idx = 0; idx < rounds; ++idx) {
                auto junkType = random::number(0, 5);

                switch (junkType) {
                    case 0: {
                        auto label = a.createLabel();
                        a.push(x86::rbp);
                        a.lea(x86::rbp, x86::qword_ptr(x86::rip, label));
                        a.xchg(x86::qword_ptr(x86::rsp), x86::rbp);
                        a.ret();
                        a.bind(label);
                    } break;
                    case 1: {
                        // i believe zasm failed to encode this?
                        constexpr std::array<uint8_t, 4> enter = {
                            0xC8,
                            0xFF,
                            0xFF,
                            0xFF  // enter 0xFFFF, 0xFF
                        };
                        // a solution to have short jmps that won't get encoded by zasm
                        constexpr std::array<uint8_t, 2> jnz = {
                            0x75,
                            0x04
                        };

                        auto label = a.createLabel();
                        a.pushfq();
                        a.pop(x86::rbp);
                        a.cmp(x86::rbp, random::number<uint16_t>());
                        a.jnz(label);

                        a.embed(enter.data(), enter.size());
                        
                        a.bind(label);
                        a.push(x86::rbp);
                        a.popfq();
                    } break;
                    case 2: {
                        auto rndBytes = random::bytes(random::number<size_t>(3, 7));
                        auto label = a.createLabel();
                        a.clc();
                        a.jnb(label);
                        a.embed(rndBytes.data(), rndBytes.size());
                        a.bind(label);
                    } break;
                    case 3: {
                        auto rndBytes = random::bytes(random::number<size_t>(3, 7));
                        auto label = a.createLabel();
                        a.stc();
                        a.jb(label);
                        a.embed(rndBytes.data(), rndBytes.size());
                        a.bind(label);
                    } break;
                    case 4: {
                        auto rnd1 = random::number<uint32_t>();
                        auto rnd2 = random::number<uint32_t>();
                        a.pushfq();

                        a.push(x86::r15);
                        a.push(x86::r14);

                        a.mov(x86::r15, rnd1);
                        a.mov(x86::r14, rnd2);

                        auto op = random::number<uint8_t>(0, 4);

                        switch (op) {
                            case 0:
                                a.add(x86::r15, x86::r14);
                                break;
                            case 1:
                                a.sub(x86::r15, x86::r14);
                                break;
                            case 2:
                                a.and_(x86::r15, x86::r14);
                                break;
                            case 3:
                                a.xor_(x86::r15, x86::r14);
                                break;
                            case 4:
                                a.imul(x86::r15, x86::r14);
                                break;
                        }

                        a.pop(x86::r14);
                        a.pop(x86::r15);

                        a.popfq();
                    } break;
                    case 5: {
                        a.push(x86::r15);
                        a.push(x86::r14);
                        a.pushfq();
                        a.push(x86::r13);
                        a.push(x86::r12);

                        a.mov(x86::r15, 0);
                        a.add(x86::r15, 1); 
                        a.sub(x86::r15, 1);

                        a.mov(x86::r14, 0xFF); 
                        a.xor_(x86::r14, x86::r14);
                        a.add(x86::r14, 2);
                        a.sub(x86::r14, 2);

                        a.mov(x86::r13, random::number<uint32_t>());
                        a.and_(x86::r13, 0);
                        a.or_(x86::r13, 0);

                        a.mov(x86::r12, x86::r15);
                        a.inc(x86::r12);
                        a.dec(x86::r12);

                        a.pop(x86::r12);
                        a.pop(x86::r13);
                        a.popfq();
                        a.pop(x86::r14);
                        a.pop(x86::r15);

                    } break;
                    default:
                        break;
                }
            }
        }
    };
}  // namespace rhield::passes