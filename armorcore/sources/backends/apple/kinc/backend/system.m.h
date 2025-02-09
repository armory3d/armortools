#include <kinc/system.h>

int kinc_hardware_threads(void) {
	return (int)[[NSProcessInfo processInfo] processorCount];
}

int kinc_cpu_cores(void) {
	return kinc_hardware_threads();
}
