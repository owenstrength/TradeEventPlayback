#include <stdint.h>

struct event // total size with padding: 12 bytes 
{
    uint32_t timestamp; // 4 bytes
    uint16_t quantity; // 2 bytes
    uint16_t price; // 2 bytes
    uint8_t type; // 1 byte
};

_Static_assert(sizeof(struct event) == 12, "event size changed"); // Non-portable by design, same compiler + same arch

