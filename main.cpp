#include "nv12_handle.h"

#define MULTITHREAD 1
#define WHOLE_DOWDSCALE 0

int main(int argc, const char *argv[])
{
    if (MULTITHREAD)
    {
        downscale_part_color_convert_thread();
        // X86-Release: downscale  + color_convert 取决于3个线程中最长执行时间的线程5.3ms
    }
    else if (WHOLE_DOWDSCALE)
    {
        downscale();
        // X86-Release: downscale + color_convert 4.8ms
    }
    else
    {
        downscale_part();
        // X86-Release: downscale  + color_convert 5.2ms
    }

    return 0;
}


