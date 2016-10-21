// for strerror_r
#define _POSIX_C_SOURCE 200809L
#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif

// for realpath
#define _BSD_SOURCE
#define _XOPEN_SOURCE_EXTENDED
