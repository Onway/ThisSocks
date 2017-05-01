#ifndef THISSOCKS_FEATURE_H_
#define THISSOCKS_FEATURE_H_

// for strerror_r
#define _POSIX_C_SOURCE 200809L
#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif

// for realpath
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#endif  // THISSOCKS_FEATURE_H_ 
