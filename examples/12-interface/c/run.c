#include <unistd.h>

#include <ptef.h>

static char *custom_colors[][2] = {
    { "PASS",  "\e[1;32mPASS\e[0m " },
    { "FAIL",  "\e[1;31mFAIL\e[0m " },
    { "RUN",   "\e[1;34mRUN\e[0m  " },
    { "ERROR", "\e[1;41mERROR\e[0m" },
    { NULL }
};

int main()
{
    ptef_status_colors = custom_colors;

    ptef_report("PASS", "some-test", 0);
    ptef_report("FAIL", "some-test", 0);
    ptef_report("RUN", "some-test", 0);
    ptef_report("ERROR", "some-test", 0);

    int fd = ptef_mklog("some-test", 0);
    char text[] = "this is some-test errout\n";
    write(fd, text, sizeof(text));
    close(fd);

    return 0;
}
