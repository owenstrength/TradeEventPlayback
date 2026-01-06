#include <stdint.h>

struct replay_state {
    uint64_t recorded_events;
    uint16_t current_price;
    uint16_t current_quantity;
    uint32_t prev_timestamp;
    uint64_t invalid_events;
};