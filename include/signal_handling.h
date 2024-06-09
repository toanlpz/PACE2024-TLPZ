#pragma once


extern volatile int running;

#ifdef _WIN32
    #include<windows.h>
    BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);
#else
    #include <signal.h>
    void sig_handler(int signum);
#endif

int enable_signal();