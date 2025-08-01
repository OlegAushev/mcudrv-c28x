#pragma once

#include <emb/scopedenum.hpp>
#include <emb/singleton.hpp>
#include <mcu/c28x/gpio.hpp>
#include <mcu/c28x/system.hpp>

namespace mcu {
namespace c28x {
namespace can {

SCOPED_ENUM_UT_DECLARE_BEGIN(Peripheral, uint32_t) {
    cana,
    canb
} SCOPED_ENUM_DECLARE_END(Peripheral)

const size_t peripheral_count = 2;

SCOPED_ENUM_UT_DECLARE_BEGIN(Bitrate, uint32_t) {
    bitrate_125k = 125000,
    bitrate_250k = 250000,
    bitrate_500k = 500000,
    bitrate_1000k = 1000000,
} SCOPED_ENUM_DECLARE_END(Bitrate)

SCOPED_ENUM_DECLARE_BEGIN(Mode) {
    normal = 0,
    silent = CAN_TEST_SILENT,
    loopback = CAN_TEST_LBACK,
    external_loopback = CAN_TEST_EXL,
    silent_loopback = CAN_TEST_SILENT | CAN_TEST_LBACK
} SCOPED_ENUM_DECLARE_END(Mode)

struct MessageObject {
    uint32_t obj_id;
    uint32_t frame_id;
    CAN_MsgFrameType frame_type;
    CAN_MsgObjType obj_type;
    uint32_t frame_idmask;
    uint32_t flags;
    uint16_t data_len;
};

struct RxPinConfig { uint32_t pin; uint32_t mux; };
struct TxPinConfig { uint32_t pin; uint32_t mux; };

struct Config {
    Bitrate bitrate;
    Mode mode;
};

namespace impl {

struct Module {
    uint32_t base;
    uint32_t pie_int_num;
    Module(uint32_t base_, uint32_t pie_int_num_)
            : base(base_), pie_int_num(pie_int_num_) {}
};

extern const uint32_t can_bases[2];
extern const uint32_t can_pie_int_nums[2];
extern const SysCtl_CPUSelPeriphInstance can_cpusel_instances[2];

} // namespace impl

class Module : public emb::singleton_array<Module, peripheral_count>,
               private emb::noncopyable {
private:
    const Peripheral peripheral_;
    impl::Module module_;
public:
    Module(Peripheral peripheral,
           const RxPinConfig& rx_pin,
           const TxPinConfig& tx_pin,
           const Config& config);
#ifdef CPU1
    static void transfer_control_to_cpu2(Peripheral peripheral,
                                         const RxPinConfig& rx_pin,
                                         const TxPinConfig& tx_pin);
#endif
    Peripheral peripheral() const { return peripheral_; }
    uint32_t base() const { return module_.base; }

    bool recv(uint32_t obj_id, uint16_t* data_buf) const {
        return CAN_readMessage(module_.base, obj_id, data_buf);
    }

    void send(uint32_t obj_id, const uint16_t* data_buf, uint16_t data_len) {
        CAN_sendMessage(module_.base, obj_id, data_len, data_buf);
    }

    void setup_message_object(const MessageObject& msg_obj) {
        CAN_setupMessageObject(module_.base,
                               msg_obj.obj_id,
                               msg_obj.frame_id,
                               msg_obj.frame_type,
                               msg_obj.obj_type,
                               msg_obj.frame_idmask,
                               msg_obj.flags,
                               msg_obj.data_len);
    }

    void register_interrupt_handler(void (*handler)(void));
    void register_interrupt_callback(void (*callback)(Module*,
                                                      uint32_t,
                                                      uint16_t));
    void enable_interrupts() { Interrupt_enable(module_.pie_int_num); }
    void disable_interrupts() { Interrupt_disable(module_.pie_int_num); }
    uint32_t interrupt_cause() const {
        return CAN_getInterruptCause(module_.base);
    }

    void acknowledge_interrupt(uint32_t interrupt_cause) {
        CAN_clearInterruptStatus(module_.base, interrupt_cause);
        CAN_clearGlobalInterruptStatus(module_.base, CAN_GLOBAL_INT_CANINT0);
        Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
    }
protected:
#ifdef CPU1
    static void init_pins(const RxPinConfig& rx_pin,
                          const TxPinConfig& tx_pin);
#endif
    static void (*on_interrupt_callbacks_[peripheral_count])(Module*,
                                                             uint32_t,
                                                             uint16_t);

    template <Peripheral::enum_type Periph>
    static interrupt void on_interrupt() {
        Module* module = Module::instance(Periph);
        uint32_t interrupt_cause = CAN_getInterruptCause(module->base());
        uint16_t status = CAN_getStatus(module->base());

        on_interrupt_callbacks_[Periph](module, interrupt_cause, status);

        module->acknowledge_interrupt(interrupt_cause);
    }
};

} // namespace can
} // namespace c28x
} // namespace mcu
