#include <mcu/c28x/gpio.hpp>

namespace mcu {
namespace c28x {
namespace gpio {

const uint32_t impl::pie_xint_nums[5] = {INT_XINT1,
                                         INT_XINT2,
                                         INT_XINT3,
                                         INT_XINT4,
                                         INT_XINT5};

const uint16_t impl::pie_xint_groups[5] = {INTERRUPT_ACK_GROUP1,
                                           INTERRUPT_ACK_GROUP1,
                                           INTERRUPT_ACK_GROUP12,
                                           INTERRUPT_ACK_GROUP12,
                                           INTERRUPT_ACK_GROUP12};

emb::array<emb::optional<uint32_t>, 16> DurationLogger::pins_;

} // namespace gpio
} // namespace c28x
} // namespace mcu
