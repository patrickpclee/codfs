#include <execinfo.h>
int main()
{
#ifndef backtrace_symbols_fd
    (void) backtrace_symbols_fd;
#endif
    ;
    return 0;
}
