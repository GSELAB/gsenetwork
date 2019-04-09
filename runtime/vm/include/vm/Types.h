#pragma once

namespace runtime {
namespace vm {

enum VM_STATUS_CODE {
    /** Execution finished with success. */
    VM_SUCCESS = 0,

    /** Generic execution failure. */
    VM_FAILURE = 1,

    /**
     * Execution terminated with REVERT opcode.
     *
     * In this case the amount of gas left MAY be non-zero and additional output
     * data MAY be provided in ::evmc_result.
     */
    VM_REVERT = 2,

    /** The execution has run out of gas. */
    VM_OUT_OF_GAS = 3,

    /**
     * The designated INVALID instruction has been hit during execution.
     *
     * The EIP-141 (https://github.com/ethereum/EIPs/blob/master/EIPS/eip-141.md)
     * defines the instruction 0xfe as INVALID instruction to indicate execution
     * abortion coming from high-level languages. This status code is reported
     * in case this INVALID instruction has been encountered.
     */
    VM_INVALID_INSTRUCTION = 4,

    /** An undefined instruction has been encountered. */
    VM_UNDEFINED_INSTRUCTION = 5,

    /**
     * The execution has attempted to put more items on the EVM stack
     * than the specified limit.
     */
    VM_STACK_OVERFLOW = 6,

    /** Execution of an opcode has required more items on the EVM stack. */
    VM_STACK_UNDERFLOW = 7,

    /** Execution has violated the jump destination restrictions. */
    VM_BAD_JUMP_DESTINATION = 8,

    /**
     * Tried to read outside memory bounds.
     *
     * An example is RETURNDATACOPY reading past the available buffer.
     */
    VM_INVALID_MEMORY_ACCESS = 9,

    /** Call depth has exceeded the limit (if any) */
    VM_CALL_DEPTH_EXCEEDED = 10,

    /** Tried to execute an operation which is restricted in static mode. */
    VM_STATIC_MODE_VIOLATION = 11,

    /**
     * A call to a precompiled or system contract has ended with a failure.
     *
     * An example: elliptic curve functions handed invalid EC points.
     */
    VM_PRECOMPILE_FAILURE = 12,

    /**
     * Contract validation has failed (e.g. due to EVM 1.5 jump validity,
     * Casper's purity checker or ewasm contract rules).
     */
    VM_CONTRACT_VALIDATION_FAILURE = 13,

    /**
     * An argument to a state accessing method has a value outside of the
     * accepted range of values.
     */
    VM_ARGUMENT_OUT_OF_RANGE = 14,

    /**
     * A WebAssembly `unreachable` instruction has been hit during exection.
     */
    VM_WASM_UNREACHABLE_INSTRUCTION = 15,

    /**
     * A WebAssembly trap has been hit during execution. This can be for many
     * reasons, including division by zero, validation errors, etc.
     */
    VM_WASM_TRAP = 16,

    /** EVM implementation generic internal error. */
    VM_INTERNAL_ERROR = -1,

    /**
     * The execution of the given code and/or message has been rejected
     * by the EVM implementation.
     *
     * This error SHOULD be used to signal that the EVM is not able to or
     * willing to execute the given code type or message.
     * If an EVM returns the ::EVMC_REJECTED status code,
     * the Client MAY try to execute it in other EVM implementation.
     * For example, the Client tries running a code in the EVM 1.5. If the
     * code is not supported there, the execution falls back to the EVM 1.0.
     */
    VM_REJECTED = -2
};
}
}