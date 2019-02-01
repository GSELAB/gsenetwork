#include <vm/VM.h>
#include <crypto/SHA3.h>

using namespace crypto;

namespace runtime {
namespace vm {

template <class S> S divWorkaround(S const& _a, S const& _b)
{
    return (S)(s512(_a) / s512(_b));
}

template <class S> S modWorkaround(S const& _a, S const& _b)
{
    return (S)(s512(_a) % s512(_b));
}

VM::VM(VMInterface& extVM): m_extVM(extVM)
{
    m_stackEnd = &m_stack[1024];
    m_SP = m_stackEnd;
    m_SPP = m_SP;
}

void VM::initEntry()
{
    m_next = &VM::interpret;
    initMetrics();
    optimize();
}

std::array<InstructionMetric, 256> VM::c_metrics;
void VM::initMetrics()
{
    static bool done =
	[]()
	{
		for (unsigned i = 0; i < 256; ++i)
		{
			InstructionInfo op = instructionInfo((Instruction)i);
			c_metrics[i].gasPriceTier = op.gasPriceTier;
			c_metrics[i].args = op.args;
			c_metrics[i].ret = op.ret;
		}
		return true;
	} ();
	(void)done;
}

u256 VM::exp256(u256 _base, u256 _exponent)
{
    using boost::multiprecision::limb_type;
	u256 result = 1;
	while (_exponent) {
		if (static_cast<limb_type>(_exponent) & 1) // If exponent is odd.
			result *= _base;
		_base *= _base;
		_exponent >>= 1;
	}
	return result;
}

void VM::copyCode(int extra)
{
    // Copy code so that it can be safely modified and extend code by
    // _extraBytes zero bytes to allow reading virtual data at the end
    // of the code without bounds checks.
    auto extendedSize = m_extVM.m_code.size() + extra;
    m_code.reserve(extendedSize);
    m_code = m_extVM.m_code;
    m_code.resize(extendedSize);
}

void VM::optimize()
{
    copyCode(33);

	size_t const nBytes = m_extVM.m_code.size();

	// build a table of jump destinations for use in verifyJumpDest
	//TRACE_STR(1, "Build JUMPDEST table")
	for (size_t pc = 0; pc < nBytes; ++pc)
	{
		Instruction op = Instruction(m_code[pc]);
		// TRACE_OP(2, pc, op);

		// make synthetic ops in user code trigger invalid instruction if run
		if (op == Instruction::PUSHC ||
			op == Instruction::JUMPC ||
			op == Instruction::JUMPCI) {
			// TRACE_OP(1, pc, op);
			m_code[pc] = (byte)Instruction::INVALID;
		}

		if (op == Instruction::JUMPDEST) {
			m_jumpDests.push_back(pc);
		} else if ((byte)Instruction::PUSH1 <= (byte)op &&
			(byte)op <= (byte)Instruction::PUSH32) {
			pc += (byte)op - (byte)Instruction::PUSH1 + 1;
		}
	}
}

void VM::create()
{
    m_next = &VM::interpret;
    // m_runGas = toInt63(m_schedule->createGas);

    // Collect arguments.
    u256 const endowment = m_SP[0]; //m_SP[0];
    u256 const initOff = m_SP[1]; //m_SP[1];
    u256 const initSize = m_SP[2]; //m_SP[2];

    u256 salt;
    if (m_OP == Instruction::CREATE2) {
        salt = m_SP[3];  //m_SP[3];
        // charge for hashing initCode = GSHA3WORD * ceil(len(init_code) / 32)
        //m_runGas += toInt63((u512{initSize} + 31) / 32 * m_schedule->sha3WordGas);
    }

    updateMem(memNeed(initOff, initSize));
    // updateIOGas();

    // Clear the return data buffer. This will not free the memory.
    m_returnResult.clear();

    if (m_extVM.balance(m_extVM.m_contract) >= endowment && m_extVM.m_depth < 1024) {
        /* Jorge
        *m_io_gas_p = m_io_gas;
        u256 createGas = *m_io_gas_p;
        if (!m_schedule->staticCallDepthLimit())
            createGas -= createGas / 64;
        u256 gas = createGas;
        */
        u256 gas;

        // Get init code. Casts are safe because the memory cost has been paid.
        auto off = static_cast<size_t>(initOff);
        auto size = static_cast<size_t>(initSize);
        bytesConstRef initCode{m_memory.data() + off, size};


        CreateResult result = m_extVM.create(endowment, gas, initCode, m_OP, salt, m_onOp);
        m_SPP[0] = (u160)result.address;  // Convert address to integer.
        m_returnResult = result.output;

        //*m_io_gas_p -= (createGas - gas);
        //m_io_gas = uint64_t(*m_io_gas_p);
    } else {
        m_SPP[0] = 0;
    }

    ++m_PC;
}

bool VM::callSetup(Arguments* params, bytesRef& output)
{
    // Make sure the params were properly initialized.
    assert(params->valueTransfer == 0);
    assert(params->apparentValue == 0);

    //m_runGas = toInt63(m_schedule->callGas);

    params->staticCall = (m_OP == Instruction::STATICCALL || m_extVM.m_staticCall);

    bool const haveValueArg = m_OP == Instruction::CALL || m_OP == Instruction::CALLCODE;

    Address destinationAddr = asAddress(m_SP[1]);// asAddress(m_SP[1]);
    if (m_OP == Instruction::CALL && !m_extVM.exists(destinationAddr))
        if (m_SP[2] > 0 || m_schedule->zeroValueTransferChargesNewAccountGas()) {
            //m_runGas += toInt63(m_schedule->callNewAccountGas);
        }

    if (haveValueArg && m_SP[2] > 0) {
        //m_runGas += toInt63(m_schedule->callValueTransferGas);
    }

    size_t const sizesOffset = haveValueArg ? 3 : 2;
    u256 inputOffset  = m_SP[sizesOffset];
    u256 inputSize    = m_SP[sizesOffset + 1];
    u256 outputOffset = m_SP[sizesOffset + 2];
    u256 outputSize   = m_SP[sizesOffset + 3];
    uint64_t inputMemNeed = memNeed(inputOffset, inputSize);
    uint64_t outputMemNeed = memNeed(outputOffset, outputSize);

    m_newMemSize = std::max(inputMemNeed, outputMemNeed);
    updateMem(m_newMemSize);

    /*
    updateIOGas();

    // "Static" costs already applied. Calculate call gas.
    if (m_schedule->staticCallDepthLimit()) {
        // With static call depth limit we just charge the provided gas amount.
        params->gas = m_SP[0];
    } else {
        // Apply "all but one 64th" rule.
        u256 maxAllowedCallGas = m_io_gas - m_io_gas / 64;
        params->gas = std::min(m_SP[0], maxAllowedCallGas);
    }

    //m_runGas = toInt63(params->gas);
    //updateIOGas();

    if (haveValueArg && m_SP[2] > 0)
        params->gas += m_schedule->callStipend;
    */

    params->codeAddress = destinationAddr;

    if (haveValueArg) {
        params->valueTransfer = m_SP[2];
        params->apparentValue = m_SP[2];
    } else if (m_OP == Instruction::DELEGATECALL)
        // Forward VALUE.
        params->apparentValue = m_extVM.m_value;

    uint64_t inOff = (uint64_t)inputOffset;
    uint64_t inSize = (uint64_t)inputSize;
    uint64_t outOff = (uint64_t)outputOffset;
    uint64_t outSize = (uint64_t)outputSize;

    if (m_extVM.balance(m_extVM.m_contract) >= params->valueTransfer && m_extVM.m_depth < 1024) {
        params->onOp = m_onOp;
        params->senderAddress = m_OP == Instruction::DELEGATECALL ? m_extVM.m_caller : m_extVM.m_contract;
        params->receiveAddress = (m_OP == Instruction::CALL || m_OP == Instruction::STATICCALL) ? params->codeAddress : m_extVM.m_contract;
        params->data = bytesConstRef(m_memory.data() + inOff, inSize);
        output = bytesRef(m_memory.data() + outOff, outSize);
        return true;
    }
    return false;
}

void VM::call()
{
    m_next = &VM::interpret;

    // TODO: Please check if that does not actually increases the stack size.
    //       That was the case before.
    unique_ptr<Arguments> callParams(new Arguments());

    // Clear the return data buffer. This will not free the memory.
    m_returnResult.clear();

    bytesRef output;
    if (callSetup(callParams.get(), output)) {
        CallResult result = m_extVM.call(*callParams);
        result.output = output.toBytes();

        // Here we have 2 options:
        // 1. Keep the whole returned memory buffer (owning_bytes_ref):
        //    higher memory footprint, no memory copy.
        // 2. Copy only the return data from the returned memory buffer:
        //    minimal memory footprint, additional memory copy.
        // Option 2 used:
        m_returnResult = result.output;

        m_SPP[0] = result.status == VM_SUCCESS ? 1 : 0;
    } else {
        m_SPP[0] = 0;
    }

    //m_io_gas += uint64_t(callParams->gas);
    ++m_PC;
}

void VM::copyToMemory(bytesConstRef data, u256* sp)
{
    auto offset = static_cast<size_t>(sp[0]);
    s512 bigIndex = sp[1];
    auto index = static_cast<size_t>(bigIndex);
    auto size = static_cast<size_t>(sp[2]);

    size_t sizeToBeCopied = bigIndex + size > data.size() ? data.size() < bigIndex ? 0 : data.size() - index : size;

    if (sizeToBeCopied > 0)
        std::memcpy(m_memory.data() + offset, data.data() + index, sizeToBeCopied);
    if (size > sizeToBeCopied)
        std::memset(m_memory.data() + offset + sizeToBeCopied, 0, size - sizeToBeCopied);
}

uint64_t VM::memNeed(u256 offset, u256 size)
{
    return toInt63(size ? u512(offset) + size : u512(0));
}

void VM::throwException(ExceptionType type)
{
    switch (type) {
    case BadInstruction:
        //BOOST_THROW_EXCEPTION(BadInstructionException("BadInstruction"));
        throw BadInstructionException("BadInstruction");
        break;
    case BadJump:
        throw BadJumpException("BadJump");
        break;
    case BadStack:
        throw BadStackException("BadStack");
        break;
    case RevertInstruction:
        throw BadInstructionException("RevertInstruction");
        break;
    case BufferOverrun:
        throw BufferOverrunException("BufferOverrun");
        break;
    case BadOverflow:
        throw BadOverflowException("BadOverflow");
        break;
    case OutOfGas:
        throw OutOfGasException("OutOfGas");
        break;
    case DisallowedStateChange:
        throw DisallowedStateChangeException("DisallowedStateChange");
    default:
        throw std::exception();
        break;
    }
}

int64_t VM::verifyJumpDest(u256 const& dest, bool th)
{
    // check for overflow
    if (dest <= 0x7FFFFFFFFFFFFFFF) {
        // check for within bounds and to a jump destination
        // use binary search of array because hashtable collisions are exploitable
        uint64_t pc = uint64_t(dest);
        if (std::binary_search(m_jumpDests.begin(), m_jumpDests.end(), pc))
            return pc;
    }
    if (th)
        throwException(BadJump);
    return -1;
}

void VM::onOperation()
{
/*
    if (m_onOp) {
        (m_onOp)(++m_nSteps, m_PC, m_OP,
            m_newMemSize > m_mem.size() ? (m_newMemSize - m_mem.size()) / 32 : uint64_t(0),
            m_runGas, m_io_gas, this, m_extVM);
    }
*/
    if (m_onOp) {
        (m_onOp)(++m_steps, m_PC, m_OP,
            m_newMemSize > m_memory.size() ? (m_newMemSize - m_memory.size()) / 32 : uint64_t(0),
            uint64_t(0), uint64_t(0), this, &m_extVM);
    }
}

void VM::adjustStack(unsigned removed, unsigned added)
{
    m_SP = m_SPP;

    // adjust stack and check bounds
    m_SPP += removed;
    if (m_stackEnd < m_SPP)
        throwException(BadStack);
    m_SPP -= added;
    if (m_SPP < m_stack)
        throwException(BadStack);
}

void VM::updateMem(uint64_t newMem)
{
    m_newMemSize = (newMem + 31) / 32 * 32;
    //updateGas();
    if (m_newMemSize > m_memory.size())
        m_memory.resize(m_newMemSize);
}

void VM::fetchInstruction()
{
    m_OP = Instruction(m_code[m_PC]);
    const InstructionMetric& metric = c_metrics[static_cast<size_t>(m_OP)];
    adjustStack(metric.args, metric.ret);

    // FEES...
    //m_runGas = toInt63(m_schedule->tierStepGas[static_cast<unsigned>(metric.gasPriceTier)]);
    m_newMemSize = m_memory.size();
    m_copyMemSize = 0;
}

uint64_t VM::decodeJumpDest(byte* const code, uint64_t& pc)
{
    // turn 2 MSB-first bytes in the code into a native-order integer
    uint64_t dest      = code[pc++];
    dest = (dest << 8) | code[pc++];
    return dest;
}

uint64_t VM::decodeJumpvDest(byte* const code, uint64_t& pcA, byte voff)
{
    // Layout of jump table in bytecode...
    //     byte opcode
    //     byte n_jumps
    //     byte table[n_jumps][2]
    //
    uint64_t pc = pcA;
    byte n = code[++pc];           // byte after opcode is number of jumps
    if (voff >= n) voff = n - 1;  // if offset overflows use default jump
    pc += voff * 2;                // adjust inout pc before index destination in table

    uint64_t dest = decodeJumpDest(code, pc);

    pcA += 1 + n * 2;               // adust inout _pc to opcode after table
    return dest;
}

void VM::exec(OnOpFunc const& onOp)
{
    m_onOp = onOp;
    m_PC = 0;

    try {
        m_next = &VM::initEntry;
        do {
            (this->*m_next)();
        } while (m_next);
    } catch (...) {
        throw;
    }
}

void VM::interpret()
{
#define ON_OP() onOperation()
#define BREAK break
#define CONTINUE continue
#define DEFAULT default
#define NEXT \
    ++m_PC; \
    break


    for (;;) {
        fetchInstruction();
        switch (m_OP) {
            case Instruction::CREATE2: {
                ON_OP();
                if (!m_schedule->haveCreate2) {
                    //throwBadInstruction();
                    throwException(BadInstruction);
                }

                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }

                m_next = &VM::create;
                BREAK;
            }
            case Instruction::CREATE: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }

                m_next = &VM::create;
                BREAK;
            }
            case Instruction::DELEGATECALL:
            case Instruction::STATICCALL:
            case Instruction::CALL:
            case Instruction::CALLCODE: {
                ON_OP();
                if (m_OP == Instruction::DELEGATECALL && !m_schedule->haveDelegateCall) {
                    throwException(BadInstruction);
                    //throwBadInstruction();
                }

                if (m_OP == Instruction::STATICCALL && !m_schedule->haveStaticCall) {
                    //throwBadInstruction();
                    throwException(BadInstruction);
                }

                if (m_OP == Instruction::CALL && m_extVM.m_staticCall && m_SP[2] != 0) {
                    throwException(DisallowedStateChange);
                    //throwDisallowedStateChange();
                }

                m_next = &VM::call;
                BREAK;
            }
            case Instruction::RETURN: {
                ON_OP();
                m_copyMemSize = 0;
                updateMem(memNeed(m_SP[0], m_SP[1]));
                //updateIOGas();

                uint64_t b = (uint64_t)m_SP[0];
                uint64_t s = (uint64_t)m_SP[1];
                m_output = owning_bytes_ref{std::move(m_memory), b, s};
                m_next = 0;
                BREAK;
            }
            case Instruction::REVERT: {
                // Pre-byzantium
                if (!m_schedule->haveRevert) {
                    throwException(BadInstruction);
                    //throwBadInstruction();
                }


                ON_OP();
                m_copyMemSize = 0;
                updateMem(memNeed(m_SP[0], m_SP[1]));
                //updateIOGas();

                uint64_t b = (uint64_t)m_SP[0];
                uint64_t s = (uint64_t)m_SP[1];
                owning_bytes_ref output{move(m_memory), b, s};
                //throwRevertInstruction(move(output));
                throwException(RevertInstruction);

                BREAK;
            }
            case Instruction::SUICIDE: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    throwException(DisallowedStateChange);
                    //throwDisallowedStateChange();
                }

                //m_runGas = toInt63(m_schedule->suicideGas);
                Address dest = asAddress(m_SP[0]);

                // After EIP158 zero-value suicides do not have to pay account creation gas.
                if (m_extVM.balance(m_extVM.m_contract) > 0 || m_schedule->zeroValueTransferChargesNewAccountGas())
                    // After EIP150 hard fork charge additional cost of sending
                    // ethers to non-existing account.
                    if (m_schedule->suicideChargesNewAccountGas() && !m_extVM.exists(dest)) {
                        //m_runGas += m_schedule->callNewAccountGas;
                    }


                //updateIOGas();
                m_extVM.suicide(dest);
                m_next = 0;

                BREAK;
            }
            case Instruction::STOP: {
                ON_OP();
                //updateIOGas();
                m_next = 0;

                BREAK;
            }

            /// instructions potentially expanding memory
            case Instruction::MLOAD: {
                ON_OP();
                updateMem(toInt63(m_SP[0]) + 32);
                //updateIOGas();

                m_SPP[0] = (u256)*(h256 const*)(m_memory.data() + (unsigned)m_SP[0]);
                NEXT;
            }
            case Instruction::MSTORE: {
                ON_OP();
                updateMem(toInt63(m_SP[0]) + 32);
                //updateIOGas();

                *(h256*)&m_memory[(unsigned)m_SP[0]] = (h256)m_SP[1];
                NEXT;
            }
            case Instruction::MSTORE8: {
                ON_OP();
                updateMem(toInt63(m_SP[0]) + 1);
                //updateIOGas();

                m_memory[(unsigned)m_SP[0]] = (byte)(m_SP[1] & 0xff);
                NEXT;
            }


            case Instruction::SHA3: {
                ON_OP();
                //m_runGas = toInt63(m_schedule->sha3Gas + (u512(m_SP[1]) + 31) / 32 * m_schedule->sha3WordGas);
                updateMem(memNeed(m_SP[0], m_SP[1]));
                //updateIOGas();

                uint64_t inOff = (uint64_t)m_SP[0];
                uint64_t inSize = (uint64_t)m_SP[1];
                m_SPP[0] = (u256)sha3(bytesConstRef(m_memory.data() + inOff, inSize));
                NEXT;
            }
            case Instruction::LOG0: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }


                //logGasMem();
                //updateIOGas();

                //m_extVM.log({}, bytesConstRef(m_memory.data() + (uint64_t)m_SP[0], (uint64_t)m_SP[1]));
                NEXT;
            }
            case Instruction::LOG1: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }

                //logGasMem();
                //updateIOGas();

                //m_extVM.log({m_SP[2]}, bytesConstRef(m_memory.data() + (uint64_t)m_SP[0], (uint64_t)m_SP[1]));
                NEXT;
            }
            case Instruction::LOG2: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }

                //logGasMem();
                //updateIOGas();

                //m_extVM.log({m_SP[2], m_SP[3]}, bytesConstRef(m_memory.data() + (uint64_t)m_SP[0], (uint64_t)m_SP[1]));
                NEXT;
            }
            case Instruction::LOG3: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }

                //logGasMem();
                //updateIOGas();

                //m_extVM.log({m_SP[2], m_SP[3], m_SP[4]}, bytesConstRef(m_memory.data() + (uint64_t)m_SP[0], (uint64_t)m_SP[1]));
                NEXT;
            }

            case Instruction::LOG4: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }

                //logGasMem();
                //updateIOGas();

                //m_extVM.log({m_SP[2], m_SP[3], m_SP[4], m_SP[5]}, bytesConstRef(m_memory.data() + (uint64_t)m_SP[0], (uint64_t)m_SP[1]));
                NEXT;
            }
            case Instruction::EXP: {
                u256 expon = m_SP[1];
                //m_runGas = toInt63(m_schedule->expGas + m_schedule->expByteGas * (32 - (h256(expon).firstBitSet() / 8)));
                ON_OP();
                //updateIOGas();

                u256 base = m_SP[0];
                m_SPP[0] = exp256(base, expon);
                NEXT;
            }

            /// ordinary instructions
            case Instruction::ADD: {
                ON_OP();
                //updateIOGas();

                //pops two items and pushes their sum mod 2^256.
                m_SPP[0] = m_SP[0] + m_SP[1];
                NEXT;
            }
            case Instruction::MUL: {
                ON_OP();
                //updateIOGas();

                //pops two items and pushes their product mod 2^256.
                m_SPP[0] = m_SP[0] * m_SP[1];
                NEXT;
            }
            case Instruction::SUB: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] - m_SP[1];
                NEXT;
            }
            case Instruction::DIV: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[1] ? divWorkaround(m_SP[0], m_SP[1]) : 0;
                NEXT;
            }
            case Instruction::SDIV: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[1] ? s2u(divWorkaround(u2s(m_SP[0]), u2s(m_SP[1]))) : 0;
                --m_SP;
                NEXT;
            }
            case Instruction::MOD: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[1] ? modWorkaround(m_SP[0], m_SP[1]) : 0;
                NEXT;
            }
            case Instruction::SMOD: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[1] ? s2u(modWorkaround(u2s(m_SP[0]), u2s(m_SP[1]))) : 0;
                NEXT;
            }
            case Instruction::NOT: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = ~m_SP[0];
                NEXT;
            }
            case Instruction::LT: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] < m_SP[1] ? 1 : 0;
                NEXT;
            }
            case Instruction::GT: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] > m_SP[1] ? 1 : 0;
                NEXT;
            }
            case Instruction::SLT: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = u2s(m_SP[0]) < u2s(m_SP[1]) ? 1 : 0;
                NEXT;
            }
            case Instruction::SGT: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = u2s(m_SP[0]) > u2s(m_SP[1]) ? 1 : 0;
                NEXT;
            }
            case Instruction::EQ: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] == m_SP[1] ? 1 : 0;
                NEXT;
            }
            case Instruction::ISZERO: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] ? 0 : 1;
                NEXT;
            }
            case Instruction::AND: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] & m_SP[1];
                NEXT;
            }
            case Instruction::OR: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] | m_SP[1];
                NEXT;
            }
            case Instruction::XOR: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] ^ m_SP[1];
                NEXT;
            }
            case Instruction::BYTE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[0] < 32 ? (m_SP[1] >> (unsigned)(8 * (31 - m_SP[0]))) & 0xff : 0;
                NEXT;
            }
            case Instruction::SHL: {
                // Pre-constantinople
                if (!m_schedule->haveBitwiseShifting) {
                    throwException(BadInstruction);
                    //throwBadInstruction();
                }

                ON_OP();
                //updateIOGas();

                if (m_SP[0] >= 256)
                    m_SPP[0] = 0;
                else
                    m_SPP[0] = m_SP[1] << unsigned(m_SP[0]);
                NEXT;
            }
            case Instruction::SHR: {
                // Pre-constantinople
                if (!m_schedule->haveBitwiseShifting) {
                    //throwBadInstruction();
                    throwException(BadInstruction);
                }


                ON_OP();
                //updateIOGas();

                if (m_SP[0] >= 256)
                    m_SPP[0] = 0;
                else
                    m_SPP[0] = m_SP[1] >> unsigned(m_SP[0]);

                NEXT;
            }
            case Instruction::SAR: {
                // Pre-constantinople
                if (!m_schedule->haveBitwiseShifting) {
                    throwException(BadInstruction);
                    //throwBadInstruction();
                }


                ON_OP();
                //updateIOGas();

                static u256 const hibit = u256(1) << 255;
                static u256 const allbits =
                    u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

                u256 shiftee = m_SP[1];
                if (m_SP[0] >= 256) {
                    if (shiftee & hibit)
                        m_SPP[0] = allbits;
                    else
                        m_SPP[0] = 0;
                } else {
                    unsigned amount = unsigned(m_SP[0]);
                    m_SPP[0] = shiftee >> amount;
                    if (shiftee & hibit)
                        m_SPP[0] |= allbits << (256 - amount);
                }
                NEXT;
            }
            case Instruction::ADDMOD: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[2] ? u256((u512(m_SP[0]) + u512(m_SP[1])) % m_SP[2]) : 0;
                NEXT;
            }
            case Instruction::MULMOD: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_SP[2] ? u256((u512(m_SP[0]) * u512(m_SP[1])) % m_SP[2]) : 0;
                NEXT;
            }
            case Instruction::SIGNEXTEND: {
                ON_OP();
                //updateIOGas();

                if (m_SP[0] < 31)
                {
                    unsigned testBit = static_cast<unsigned>(m_SP[0]) * 8 + 7;
                    u256& number = m_SP[1];
                    u256 mask = ((u256(1) << testBit) - 1);
                    if (boost::multiprecision::bit_test(number, testBit))
                        number |= ~mask;
                    else
                        number &= mask;
                }
                NEXT;
            }
            case Instruction::ADDRESS: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = fromAddress(m_extVM.m_contract);
                NEXT;
            }
            case Instruction::ORIGIN: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = fromAddress(m_extVM.m_origin);
                NEXT;
            }
            case Instruction::BALANCE: {
                //m_runGas = toInt63(m_schedule->balanceGas);
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.balance(asAddress(m_SP[0]));
                NEXT;
            }
            case Instruction::CALLER: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = fromAddress(m_extVM.m_caller);
                NEXT;
            }
            case Instruction::CALLVALUE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.m_value;
                NEXT;
            }
            case Instruction::CALLDATALOAD: {
                ON_OP();
                //updateIOGas();

                if (u512(m_SP[0]) + 31 < m_extVM.m_data.size())
                    m_SP[0] = (u256)*(h256 const*)(m_extVM.m_data.data() + (size_t)m_SP[0]);
                else if (m_SP[0] >= m_extVM.m_data.size())
                    m_SP[0] = u256(0);
                else
                { 	h256 r;
                    for (uint64_t i = (uint64_t)m_SP[0], e = (uint64_t)m_SP[0] + (uint64_t)32, j = 0; i < e; ++i, ++j)
                        r[j] = i < m_extVM.m_data.size() ? m_extVM.m_data[i] : 0;
                    m_SP[0] = (u256)r;
                };
                NEXT;
            }
            case Instruction::CALLDATASIZE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.m_data.size();
                NEXT;
            }
            case Instruction::RETURNDATASIZE: {
                if (!m_schedule->haveReturnData) {
                    //throwBadInstruction();
                    throwException(BadInstruction);
                }


                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_returnResult.size();
                NEXT;
            }
            case Instruction::CODESIZE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.m_code.size();
                NEXT;
            }
            case Instruction::EXTCODESIZE: {
                //m_runGas = toInt63(m_schedule->extcodesizeGas);
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.codeSizeAt(asAddress(m_SP[0]));
                NEXT;
            }
            case Instruction::CALLDATACOPY: {
                ON_OP();
                m_copyMemSize = toInt63(m_SP[2]);
                updateMem(memNeed(m_SP[0], m_SP[2]));
                //updateIOGas();

                copyToMemory(m_extVM.m_data, m_SP);
                NEXT;
            }
            case Instruction::RETURNDATACOPY: {
                ON_OP();
                if (!m_schedule->haveReturnData) {
                    throwException(BadInstruction);
                    //throwBadInstruction();
                }

                bigint const endOfAccess = bigint(m_SP[1]) + bigint(m_SP[2]);
                if (m_returnResult.size() < endOfAccess) {
                    throwException(BufferOverrun);
                    //throwBufferOverrun(endOfAccess);
                }


                m_copyMemSize = toInt63(m_SP[2]);
                updateMem(memNeed(m_SP[0], m_SP[2]));
                //updateIOGas();

                copyToMemory(&m_returnResult, m_SP);
                NEXT;
            }
            case Instruction::EXTCODEHASH: {
                ON_OP();
                if (!m_schedule->haveExtcodehash) {
                    throwException(BadInstruction);
                    //throwBadInstruction();
                }


                //m_runGas = toInt63(m_schedule->extcodehashGas);
                //updateIOGas();

                m_SPP[0] = u256{m_extVM.codeHashAt(asAddress(m_SP[0]))};
                NEXT;
            }
            case Instruction::CODECOPY: {
                ON_OP();
                m_copyMemSize = toInt63(m_SP[2]);
                updateMem(memNeed(m_SP[0], m_SP[2]));
                //updateIOGas();

                copyToMemory(&m_extVM.m_code, m_SP);
                NEXT;
            }
            case Instruction::EXTCODECOPY: {
                ON_OP();
                //m_runGas = toInt63(m_schedule->extcodecopyGas);
                m_copyMemSize = toInt63(m_SP[3]);
                updateMem(memNeed(m_SP[1], m_SP[3]));
                //updateIOGas();

                Address a = asAddress(m_SP[0]);
                copyToMemory(&m_extVM.codeAt(a), m_SP + 1);
                NEXT;
            }
            case Instruction::GASPRICE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = 0; //m_extVM.gasPrice;
                NEXT;
            }
            case Instruction::BLOCKHASH: {
                ON_OP();
                //m_runGas = toInt63(m_schedule->blockhashGas);
                //updateIOGas();

                m_SPP[0] = (u256)m_extVM.blockHash(m_SP[0]);
                NEXT;
            }
            case Instruction::COINBASE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = (u160)m_extVM.envInfo().author();
                NEXT;
            }
            case Instruction::TIMESTAMP: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.envInfo().timestamp();
                NEXT;
            }
            case Instruction::NUMBER: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.envInfo().number();
                NEXT;
            }
            case Instruction::DIFFICULTY: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = 0; //m_extVM.envInfo().difficulty();
                NEXT;
            }
            case Instruction::GASLIMIT: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = 0; //m_extVM.envInfo().gasLimit();
                NEXT;
            }
            case Instruction::POP: {
                ON_OP();
                //updateIOGas();

                --m_SP;
                NEXT;
            }

            case Instruction::PUSH1: {
                ON_OP();
                //updateIOGas();
                ++m_PC;
                m_SPP[0] = m_code[m_PC];
                ++m_PC;
                CONTINUE;
            }

            case Instruction::PUSH2:
            case Instruction::PUSH3:
            case Instruction::PUSH4:
            case Instruction::PUSH5:
            case Instruction::PUSH6:
            case Instruction::PUSH7:
            case Instruction::PUSH8:
            case Instruction::PUSH9:
            case Instruction::PUSH10:
            case Instruction::PUSH11:
            case Instruction::PUSH12:
            case Instruction::PUSH13:
            case Instruction::PUSH14:
            case Instruction::PUSH15:
            case Instruction::PUSH16:
            case Instruction::PUSH17:
            case Instruction::PUSH18:
            case Instruction::PUSH19:
            case Instruction::PUSH20:
            case Instruction::PUSH21:
            case Instruction::PUSH22:
            case Instruction::PUSH23:
            case Instruction::PUSH24:
            case Instruction::PUSH25:
            case Instruction::PUSH26:
            case Instruction::PUSH27:
            case Instruction::PUSH28:
            case Instruction::PUSH29:
            case Instruction::PUSH30:
            case Instruction::PUSH31:
            case Instruction::PUSH32: {
                ON_OP();
                //updateIOGas();

                int numBytes = (int)m_OP - (int)Instruction::PUSH1 + 1;
                m_SPP[0] = 0;
                // Construct a number out of PUSH bytes.
                // This requires the code has been copied and extended by 32 zero
                // bytes to handle "out of code" push data here.
                for (++m_PC; numBytes--; ++m_PC)
                    m_SPP[0] = (m_SPP[0] << 8) | m_code[m_PC];

                CONTINUE;
            }
            case Instruction::JUMP: {
                ON_OP();
                //updateIOGas();
                m_PC = verifyJumpDest(m_SP[0]);
                CONTINUE;
            }
            case Instruction::JUMPI: {
                ON_OP();
                //updateIOGas();
                if (m_SP[1])
                    m_PC = verifyJumpDest(m_SP[0]);
                else
                    ++m_PC;

                CONTINUE;
            }

            case Instruction::DUP1:
            case Instruction::DUP2:
            case Instruction::DUP3:
            case Instruction::DUP4:
            case Instruction::DUP5:
            case Instruction::DUP6:
            case Instruction::DUP7:
            case Instruction::DUP8:
            case Instruction::DUP9:
            case Instruction::DUP10:
            case Instruction::DUP11:
            case Instruction::DUP12:
            case Instruction::DUP13:
            case Instruction::DUP14:
            case Instruction::DUP15:
            case Instruction::DUP16: {
                ON_OP();
                //updateIOGas();

                unsigned n = (unsigned)m_OP - (unsigned)Instruction::DUP1;
                *(uint64_t*)m_SPP = *(uint64_t*)(m_SP + n);

                // the stack slot being copied into may no longer hold a u256
                // so we construct a new one in the memory, rather than assign
                new(m_SPP) u256(m_SP[n]);
                NEXT;
            }

            case Instruction::SWAP1:
            case Instruction::SWAP2:
            case Instruction::SWAP3:
            case Instruction::SWAP4:
            case Instruction::SWAP5:
            case Instruction::SWAP6:
            case Instruction::SWAP7:
            case Instruction::SWAP8:
            case Instruction::SWAP9:
            case Instruction::SWAP10:
            case Instruction::SWAP11:
            case Instruction::SWAP12:
            case Instruction::SWAP13:
            case Instruction::SWAP14:
            case Instruction::SWAP15:
            case Instruction::SWAP16: {
                ON_OP();
                //updateIOGas();

                unsigned n = (unsigned)m_OP - (unsigned)Instruction::SWAP1 + 1;
                std::swap(m_SP[0], m_SP[n]);
                NEXT;
            }
            case Instruction::SLOAD: {
                //m_runGas = toInt63(m_schedule->sloadGas);
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_extVM.store(m_SP[0]);
                NEXT;
            }
            case Instruction::SSTORE: {
                ON_OP();
                if (m_extVM.m_staticCall) {
                    //throwDisallowedStateChange();
                    throwException(DisallowedStateChange);
                }


                //updateSSGas();
                //updateIOGas();

                m_extVM.setStore(m_SP[0], m_SP[1]);
                NEXT;
            }
            case Instruction::PC: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_PC;
                NEXT;
            }
            case Instruction::MSIZE: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = m_memory.size();
                NEXT;
            }
            case Instruction::GAS: {
                ON_OP();
                //updateIOGas();

                m_SPP[0] = 0; //m_io_gas;
                NEXT;
            }
            case Instruction::JUMPDEST: {
                //m_runGas = 1;
                ON_OP();
                //updateIOGas();

                NEXT;
            }
            case Instruction::INVALID:
            DEFAULT: {
                throwException(BadInstruction);
                //throwBadInstruction();
            }
        }
    }
}
}
}