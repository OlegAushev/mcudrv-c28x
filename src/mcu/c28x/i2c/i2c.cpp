#include <mcu/c28x/i2c.hpp>


namespace mcu {

namespace c28x {

namespace i2c {


const uint32_t impl::i2c_bases[2] = {I2CA_BASE, I2CB_BASE};


Module::Module(Peripheral peripheral, const SdaPinConfig& sda_pin, const SclPinConfig& scl_pin, const i2c::Config& config)
        : emb::singleton_array<Module, peripheral_count>(this, peripheral.underlying_value())
        , _peripheral(peripheral)
        , _module(impl::i2c_bases[peripheral.underlying_value()]) {
#ifdef CPU1
    _init_pins(sda_pin, scl_pin);
#endif
    I2C_disableModule(_module.base);

    I2C_initMaster(_module.base, mcu::c28x::sysclk_freq(), config.bitrate,
            static_cast<I2C_DutyCycle>(config.duty_cycle.underlying_value()));
    I2C_setBitCount(_module.base,
                    static_cast<I2C_BitCount>(
                            config.bitcount.underlying_value()));
    I2C_setSlaveAddress(_module.base, config.slave_addr);
    I2C_setEmulationMode(_module.base, I2C_EMULATION_FREE_RUN);

    I2C_disableFIFO(_module.base);
    I2C_enableModule(_module.base);
}


#ifdef CPU1
void Module::transfer_control_to_cpu2(Peripheral peripheral, const SdaPinConfig& sda_pin, const SclPinConfig& scl_pin) {
    _init_pins(sda_pin, scl_pin);
    GPIO_setMasterCore(sda_pin.pin, GPIO_CORE_CPU2);
    GPIO_setMasterCore(scl_pin.pin, GPIO_CORE_CPU2);

    SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL7_I2C, peripheral.underlying_value()+1, SYSCTL_CPUSEL_CPU2);
}
#endif


#ifdef CPU1
void Module::_init_pins(const SdaPinConfig& sda_pin, const SclPinConfig& scl_pin) {
    GPIO_setPadConfig(sda_pin.pin, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(sda_pin.pin, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(sda_pin.mux);

    GPIO_setPadConfig(scl_pin.pin, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(scl_pin.pin, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(scl_pin.mux);
}
#endif


} // namespace i2c

} // namespace c28x

} // namespace mcu
