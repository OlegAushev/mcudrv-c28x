#pragma once

#include <mcu/c28x/system.hpp>
#include <emb/scopedenum.hpp>
#include "F2837xD_Ipc_drivers.h"

namespace mcu {
namespace c28x {
namespace ipc {

SCOPED_ENUM_DECLARE_BEGIN(Cpu) {
    cpu1,
    cpu2,
} SCOPED_ENUM_DECLARE_END(Cpu)

#if defined(CPU1)
const Cpu this_cpu = Cpu::cpu1;
#elif defined(CPU2)
const Cpu this_cpu = Cpu::cpu2;
#else
#error "CPU is not defined."
#endif

SCOPED_ENUM_DECLARE_BEGIN(FlagType) {
    local,
    remote,
} SCOPED_ENUM_DECLARE_END(FlagType)


class Flag {
private:
    const uint32_t bitmask_;
    const Cpu master_cpu_;
    const FlagType type_;
public:
    Flag(uint32_t flag_id, Cpu master_cpu)
            : bitmask_(uint32_t(1) << flag_id),
              master_cpu_(master_cpu),
              type_((this_cpu == master_cpu_) ?
                    FlagType::local : FlagType::remote) {
        assert(flag_id < 32);
    }

    FlagType type() const { return type_; }
    bool is_local() const { return type_ == FlagType::local; }
    bool is_remote() const { return type_ == FlagType::remote; }

    void set() {
        assert(is_local());
        if (is_local()) {
            IPCLtoRFlagSet(bitmask_);
        }
    }

    void clear() {
        if (is_local()) {
            IPCLtoRFlagClear(bitmask_);
        } else {
            IPCRtoLFlagAcknowledge(bitmask_);
        }
    }

    bool is_set() const {
        if (is_local()) {
            return IPCLtoRFlagBusy(bitmask_);
        } else {
            return IPCRtoLFlagBusy(bitmask_);
        }
    }

    void wait_until_set() const {
        while (!is_set()) {
            // wait
        }
    }
};

SCOPED_ENUM_UT_DECLARE_BEGIN(InterruptType, uint32_t) {
    ipc_interrupt0 = INT_IPC_0,
    ipc_interrupt1 = INT_IPC_1,
    ipc_interrupt2 = INT_IPC_2,
    ipc_interrupt3 = INT_IPC_3,
} SCOPED_ENUM_DECLARE_END(InterruptType)

inline void registerIpcInterruptHandler(InterruptType ipc_interrupt, void (*handler)(void)) {
    Interrupt_register(ipc_interrupt.underlying_value(), handler);
    Interrupt_enable(ipc_interrupt.underlying_value());
}

} // namespace ipc
} // namespace c28x
} // namespace mcu
