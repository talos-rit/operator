#include <stdint.h>
#include <sys/time.h>

static inline uint16_t tval_diff_ms(struct timeval* end, struct timeval* start)
{
    time_t start_ms = (start->tv_sec * 1000) + (start->tv_usec / 1000);
    time_t end_ms = (end->tv_sec * 1000) + (end->tv_usec / 1000);
    return end_ms - start_ms;
}
