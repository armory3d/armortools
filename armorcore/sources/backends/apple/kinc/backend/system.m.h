#include <kinc/system.h>

int kinc_hardware_threads(void) {
	return (int)[[NSProcessInfo processInfo] processorCount];
}

#ifdef KINC_APPLE_SOC

int kinc_cpu_cores(void) {
	return kinc_hardware_threads();
}

#else

#include <sys/sysctl.h>

int kinc_cpu_cores(void) {
	uint32_t proper_cpu_count = 1;
	size_t count_length = sizeof(count_length);
	sysctlbyname("hw.physicalcpu", &proper_cpu_count, &count_length, 0, 0);
	return (int)proper_cpu_count;
}

#endif
