/* getRSS.h */

#ifndef GETRSS_H
#define GETRSS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t getPeakRSS();
size_t getCurrentRSS();

#ifdef __cplusplus
}
#endif

#endif
