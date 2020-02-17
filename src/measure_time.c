unsigned long long measure_time_now_ms(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return 1000ULL * tv.tv_sec + (tv.tv_usec / 1000ULL);
}

unsigned long long measure_time_now_us(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return 1000000ULL * tv.tv_sec + (tv.tv_usec);
}
