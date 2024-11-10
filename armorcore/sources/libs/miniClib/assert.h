#pragma once

#ifdef NDEBUG
#define assert(condition)
#else
static void assert(int condition) {}
#endif
