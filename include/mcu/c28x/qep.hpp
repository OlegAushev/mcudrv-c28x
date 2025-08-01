#pragma once


#include <mcu/c28x/system.hpp>
#include <mcu/c28x/gpio.hpp>
#include <emb/optional.hpp>
#include <emb/scopedenum.hpp>
#include <emb/singleton.hpp>


namespace mcu {

namespace c28x {

namespace qep {


SCOPED_ENUM_UT_DECLARE_BEGIN(Peripheral, uint32_t) {
    qep1,
    qep2,
    qep3
} SCOPED_ENUM_DECLARE_END(Peripheral)


const size_t peripheral_count = 3;


SCOPED_ENUM_DECLARE_BEGIN(InputMode) {
    quadrature = EQEP_CONFIG_QUADRATURE,
    clockdir = EQEP_CONFIG_CLOCK_DIR,
    upcount = EQEP_CONFIG_UP_COUNT,
    downcount = EQEP_CONFIG_DOWN_COUNT
} SCOPED_ENUM_DECLARE_END(InputMode)


SCOPED_ENUM_DECLARE_BEGIN(Resolution) {
    x2 = EQEP_CONFIG_2X_RESOLUTION,
    x1 = EQEP_CONFIG_1X_RESOLUTION
} SCOPED_ENUM_DECLARE_END(Resolution)


SCOPED_ENUM_DECLARE_BEGIN(SwapAB) {
    disabled = EQEP_CONFIG_NO_SWAP,
    enabled = EQEP_CONFIG_SWAP
} SCOPED_ENUM_DECLARE_END(SwapAB)


SCOPED_ENUM_DECLARE_BEGIN(PositionResetMode) {
    reset_on_idx = EQEP_POSITION_RESET_IDX,
    reset_on_max = EQEP_POSITION_RESET_MAX_POS,
    reset_on_1st_idx = EQEP_POSITION_RESET_1ST_IDX,
    reset_on_timeout = EQEP_POSITION_RESET_UNIT_TIME_OUT
} SCOPED_ENUM_DECLARE_END(PositionResetMode)


struct QepaPinConfig { uint32_t pin; uint32_t mux; };
struct QepbPinConfig { uint32_t pin; uint32_t mux; };
struct QepiPinConfig { uint32_t pin; uint32_t mux; };


struct Config {
    InputMode input_mode;
    Resolution resolution;
    SwapAB swap_ab;
    PositionResetMode reset_mode;
    uint32_t max_position;
    uint32_t timeout_freq;
    uint32_t latch_mode;
    uint16_t init_mode;
    uint32_t init_position;
    EQEP_CAPCLKPrescale cap_prescaler;
    EQEP_UPEVNTPrescale event_prescaler;
    uint16_t int_flags;
};


namespace impl {

struct Module {
    uint32_t base;
    uint16_t int_flags;
    uint32_t pie_int_num;
    Module(uint32_t base_, uint16_t int_flags_, uint32_t pie_int_num_)
            : base(base_), int_flags(int_flags_), pie_int_num(pie_int_num_) {}
};


extern const uint32_t qep_bases[3];
extern const uint32_t qep_pie_int_nums[3];

} // namespace impl


class Module : public emb::singleton_array<Module, peripheral_count>, private emb::noncopyable {
private:
    const Peripheral _peripheral;
    impl::Module _module;
public:
    Module(Peripheral peripheral,
           const QepaPinConfig& qepa_pin, emb::optional<QepbPinConfig> qepb_pin, emb::optional<QepiPinConfig> qepi_pin,
           const Config& config);

    Peripheral peripheral() const { return _peripheral; }
    uint32_t base() const { return _module.base; }
    void register_interrupt_handler(void (*handler)(void)) {
        Interrupt_register(_module.pie_int_num, handler);
        EQEP_enableInterrupt(_module.base, _module.int_flags);
    }
    void enable_interrupts() { Interrupt_enable(_module.pie_int_num); }
    void disable_interrupts() { Interrupt_disable(_module.pie_int_num); }
protected:
#ifdef CPU1
    static void _init_pins(const QepaPinConfig& qepa_pin, emb::optional<QepbPinConfig> qepb_pin, emb::optional<QepiPinConfig> qepi_pin);
#endif
};


} // namespace qep

} // namespace c28x

} // namespace mcu
