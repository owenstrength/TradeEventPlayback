#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../shared/event.h"
#include "../shared/event_file_header.h"

#define MAX_EVENTS 10000000
#define VERSION 1

void write_header_to_file(FILE *f, uint32_t version, uint64_t event_count) {
    struct header hdr;
    hdr.version = version;
    hdr.event_count = event_count;

    size_t written = fwrite(&hdr, sizeof(struct header), 1, f);
    if (written != 1) {
        fprintf(stderr, "fwrite header failed\n");
        exit(EXIT_FAILURE);
    }
}

/* 
This functions writes to a binary file an array of events.
The event structure is defined in shared/event.h as:
struct event
{
    uint32_t timestamp;
    uint16_t quantity;
    uint16_t price;
    uint8_t type;
};

*/
void write_events_to_file(const char *filename,
                          const struct event *restrict events,
                          size_t count)
{
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return;
    }
    write_header_to_file(f, VERSION, count);

    size_t written = fwrite(events, sizeof(struct event), count, f);
    if (written != count) {
        fprintf(stderr, "fwrite failed: wrote %zu of %zu\n", written, count);
    }

    if (fclose(f) != 0) {
        perror("fclose");
    }
}

struct event* create_sample_events(size_t *out_count) {
    struct event* sample_events = malloc(MAX_EVENTS * sizeof(struct event));
    if (!sample_events) {
        perror("malloc");
        return NULL;
    }
    for (size_t i = 0; i < MAX_EVENTS; i++) {
        memset(&sample_events[i], 0, sizeof(struct event));
        sample_events[i].timestamp = (uint32_t)i;
        sample_events[i].quantity = (uint16_t)(i % 100);
        sample_events[i].price = (uint16_t)((i % 1000) + 1000);
        sample_events[i].type = (uint8_t)(i % 4);
    }
    *out_count = MAX_EVENTS;
    return sample_events;
}

int main() {

    size_t event_count;
    struct event *events = create_sample_events(&event_count);
    if (!events) {
        return 1;
    }
    write_events_to_file("events.bin", events, event_count);
    free(events);
    return 0;
}
