#pragma once

/*! \file usage.h
    \brief Provides the usage enum.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g4_usage {
    KINC_G4_USAGE_STATIC,
    KINC_G4_USAGE_DYNAMIC,
    KINC_G4_USAGE_READABLE
} kinc_g4_usage_t;

#ifdef __cplusplus
}
#endif
