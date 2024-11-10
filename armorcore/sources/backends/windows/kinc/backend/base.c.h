#ifdef KINC_NO_CLIB

#ifndef NDEBUG
void _wassert(wchar_t const *message, wchar_t const *filename, unsigned line) {
	__debugbreak();
}

void _RTC_CheckStackVars(void) {}

void _RTC_InitBase(void) {}

void _RTC_Shutdown(void) {}

void _RTC_AllocaHelper(void) {}

void _RTC_CheckStackVars2(void) {}

void __GSHandlerCheck(void) {}

void __fastcall __security_check_cookie(_In_ uintptr_t _StackCookie) {}

uintptr_t __security_cookie;

int _fltused = 1;

void __report_rangecheckfailure(void) {}

void __chkstk(void) {}
#endif

#endif
