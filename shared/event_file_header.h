#include <stdint.h>

struct header {
    uint64_t event_count;
    uint32_t version;
};

_Static_assert(sizeof(struct header) == 16, "header size changed"); // Non-portable by design, same compiler + same arch
