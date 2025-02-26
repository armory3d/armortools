#pragma once

typedef struct {
	void *DebugInfo;
	long LockCount;
	long RecursionCount;
	void *OwningThread;
	void *LockSemaphore;
	unsigned long __w64 SpinCount;
} kinc_microsoft_critical_section_t;

typedef struct {
	kinc_microsoft_critical_section_t criticalSection;
} kinc_mutex_impl_t;

typedef struct {
	void *id;
} kinc_uber_mutex_impl_t;
