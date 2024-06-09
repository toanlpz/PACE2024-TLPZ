#include "signal_handling.h"

#include <stdio.h>

volatile int running = 1;


#ifdef _WIN32
    BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
        switch (fdwCtrlType) {

            case CTRL_C_EVENT:
            case CTRL_BREAK_EVENT:
                fflush(stdout);
                fflush(stderr);
                running = 0;
                return TRUE;

            default:
                return FALSE;
        }
    }
#else
    void sig_handler(int signum)
    {
        signal(SIGTERM, sig_handler);
        signal(SIGINT, sig_handler);

        fprintf(stderr, "SIGTERM / SIGINT Process will now terminates.\n");

        fflush(stdout);
        fflush(stderr);
        running = 0;
    }
#endif


int enable_signal()
{
    #ifdef _WIN32
        if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
            fprintf(stderr, "Error setting control handler\n");
            return 1;
        }
    #else
        signal(SIGTERM, sig_handler);
        signal(SIGINT, sig_handler);
    #endif
    return 0;
}