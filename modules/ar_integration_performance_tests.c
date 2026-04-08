#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_methodology.h"
#include "ar_system.h"

#define NUM_AGENTS 100
#define NUM_MESSAGES 1000

typedef struct {
    struct timespec start;
    struct timespec end;
} ar_timer_t;

static void ar_timer__start(ar_timer_t *mut_timer) {
    AR_ASSERT(mut_timer != NULL, "Timer must exist");
    clock_gettime(CLOCK_MONOTONIC, &mut_timer->start);
}

static void ar_timer__stop(ar_timer_t *mut_timer) {
    AR_ASSERT(mut_timer != NULL, "Timer must exist");
    clock_gettime(CLOCK_MONOTONIC, &mut_timer->end);
}

static uint64_t ar_timer__elapsed_us(const ar_timer_t *ref_timer) {
    int64_t seconds;
    int64_t nanoseconds;

    AR_ASSERT(ref_timer != NULL, "Timer must exist");

    seconds = (int64_t)(ref_timer->end.tv_sec - ref_timer->start.tv_sec);
    nanoseconds = ref_timer->end.tv_nsec - ref_timer->start.tv_nsec;
    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += 1000000000L;
    }

    return (uint64_t)(seconds * 1000000LL) + (uint64_t)(nanoseconds / 1000L);
}

int main(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_data_t *own_shared_context;
    int64_t agent_ids[NUM_AGENTS];
    ar_timer_t timer;
    int processed;
    int i;

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    if (own_system == NULL) {
        return 1;
    }

    own_shared_context = ar_data__create_map();
    AR_ASSERT(own_shared_context != NULL, "Shared context allocation should succeed");
    if (own_shared_context == NULL) {
        ar_system__destroy(own_system);
        return 1;
    }

    (void)ar_system__init(own_system, NULL, NULL);

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should provide an agency");
    if (mut_agency == NULL) {
        ar_data__destroy(own_shared_context);
        ar_system__destroy(own_system);
        return 1;
    }

    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should provide a methodology");
    if (mut_methodology == NULL) {
        ar_data__destroy(own_shared_context);
        ar_system__destroy(own_system);
        return 1;
    }

    if (!ar_methodology__create_method(mut_methodology,
                                       "ring-node",
                                       "send(memory.next_id, message)",
                                       "1.0.0")) {
        fprintf(stderr, "Failed to create ring benchmark method\n");
        ar_data__destroy(own_shared_context);
        ar_system__destroy(own_system);
        return 1;
    }

    for (i = 0; i < NUM_AGENTS; i++) {
        agent_ids[i] = ar_agency__create_agent(mut_agency, "ring-node", "1.0.0", own_shared_context);
        if (agent_ids[i] == 0) {
            fprintf(stderr, "Failed to create benchmark agent %d\n", i);
            ar_data__destroy(own_shared_context);
            ar_system__destroy(own_system);
            return 1;
        }
    }

    for (i = 0; i < NUM_AGENTS; i++) {
        ar_data_t *mut_memory;
        int next_id;

        mut_memory = ar_agency__get_agent_mutable_memory(mut_agency, agent_ids[i]);
        AR_ASSERT(mut_memory != NULL, "Agent memory should exist");
        if (mut_memory == NULL) {
            fprintf(stderr, "Failed to get memory for benchmark agent %d\n", i);
            ar_data__destroy(own_shared_context);
            ar_system__destroy(own_system);
            return 1;
        }

        next_id = (int)agent_ids[(i + 1) % NUM_AGENTS];
        if (!ar_data__set_map_integer(mut_memory, "next_id", next_id)) {
            fprintf(stderr, "Failed to set next_id for benchmark agent %d\n", i);
            ar_data__destroy(own_shared_context);
            ar_system__destroy(own_system);
            return 1;
        }
    }

    {
        ar_data_t *own_message = ar_data__create_integer(1);
        AR_ASSERT(own_message != NULL, "Initial message allocation should succeed");
        if (own_message == NULL) {
            ar_data__destroy(own_shared_context);
            ar_system__destroy(own_system);
            return 1;
        }

        if (!ar_agency__send_to_agent(mut_agency, agent_ids[0], own_message)) {
            fprintf(stderr, "Failed to send initial benchmark message\n");
            ar_data__destroy(own_shared_context);
            ar_system__destroy(own_system);
            return 1;
        }
    }

    ar_timer__start(&timer);

    processed = 0;
    while (processed < NUM_MESSAGES) {
        if (!ar_system__process_next_message(own_system)) {
            fprintf(stderr, "Benchmark message loop stopped early after %d messages\n", processed);
            ar_data__destroy(own_shared_context);
            ar_system__destroy(own_system);
            return 1;
        }
        processed++;
    }

    ar_timer__stop(&timer);

    printf("METRIC total_µs=%llu\n", (unsigned long long)ar_timer__elapsed_us(&timer));

    ar_data__destroy(own_shared_context);
    ar_system__destroy(own_system);
    return 0;
}
