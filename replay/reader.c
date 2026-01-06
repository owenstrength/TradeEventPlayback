#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "../shared/event.h"
#include "../shared/event_file_header.h"
#include "replay_state.h"

#define VERSION 1

struct replay_state state = {0};

void reset_state() {
    state.recorded_events = 0;
    state.current_price = 0;
    state.current_quantity = 0;
    state.prev_timestamp = 0;
    state.invalid_events = 0;
}

void process_event(struct event *e) {
    state.recorded_events++;
    state.current_price = e->price;
    state.current_quantity = e->quantity;
    if (e->timestamp < state.prev_timestamp) {
        state.invalid_events++;
    }
    state.prev_timestamp = e->timestamp;
}

int validate_header_and_layout(struct stat *st, struct header *hdr) {
    if (st->st_size < sizeof(struct header)) {
        fprintf(stderr, "File too small for header\n");
        return 1;
    }
    
    size_t data_size = st->st_size - sizeof(struct header);
    if (data_size % sizeof(struct event) != 0) {
        fprintf(stderr, "Event data size not aligned\n");
        return 1;
    }
    
    size_t computed_event_count = data_size / sizeof(struct event);
    if (hdr->event_count != computed_event_count) {
        fprintf(stderr, "Event count mismatch: header=%llu computed=%zu\n", 
                hdr->event_count, computed_event_count);
        return 1;
    }

    if (hdr->version != VERSION) {
        fprintf(stderr, "Version mismatch: expected=%u found=%u\n", 
                VERSION, hdr->version);
        return 1;
    }

    fprintf(stdout, "File validated: version=%u event_count=%llu\n", 
           hdr->version, hdr->event_count);
    return 0;
}

void print_summary() {
    printf("Total recorded events: %llu\n", state.recorded_events);
    printf("Last known price: %u\n", state.current_price);
    printf("Last known quantity: %u\n", state.current_quantity);
    printf("Invalid events encountered: %llu\n", state.invalid_events);
}

void replay_events_mmap(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct stat st;
    if(fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return;
    }

    if (st.st_size < sizeof(struct header)) {
        fprintf(stderr, "File too small for header\n");
        close(fd);
        return;
    }
    
    size_t size = st.st_size - sizeof(struct header);

    struct event *events_base = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (events_base == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    struct header *hdr = (struct header *)events_base;
    int valid_state = validate_header_and_layout(&st, hdr);
    if (valid_state != 0) {
        munmap(events_base, st.st_size);
        close(fd);
        return;
    }

    struct event *events = (struct event *)((uint8_t *)events_base + sizeof(struct header));
    size_t event_count = size / sizeof(struct event);
    for (size_t i = 0; i < event_count; i++) {
        process_event(&events[i]);
    }

    munmap(events_base, st.st_size);

    close(fd);

    print_summary();
}

int main() {
    reset_state();
    replay_events_mmap("events.bin");
    return 0;
}