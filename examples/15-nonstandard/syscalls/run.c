#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <ptef.h>

int test_stat(void)
{
    struct stat buf;
    return stat("/etc/passwd", &buf);
}

int test_access(void)
{
    return access("/etc/passwd", F_OK);
}

int test_open(void)
{
    int fd = open("/etc/passwd", O_RDWR | O_APPEND);
    if (fd != -1)
        close(fd);
    return fd;
}

#define TEST(name) { test_ ## name, #name }
struct test {
    int (*func)(void);
    char *name;
} all_tests[] = {
    TEST(stat),
    TEST(access),
    TEST(open),
};
#define all_tests_cnt (sizeof(all_tests)/sizeof(struct test))
void run_test(struct test *t)
{
    int logfd;
    if ((logfd = ptef_mklog(t->name, 0)) == -1)
        return;
    if (t->func() == -1) {
        dprintf(logfd, "errno %d: %s\n", errno, strerror(errno));
        ptef_report("FAIL", t->name, 0);
    } else {
        ptef_report("PASS", t->name, 0);
    }
    close(logfd);
}

int main(int argc, char **argv)
{
    int i;
    unsigned long j;

    if (argc < 2) {
        // run all tests
        for (j = 0; j < all_tests_cnt; j++)
            run_test(&all_tests[j]);
    } else {
        // run only cmdline-specified tests
        for (i = 0; i < argc; i++)
            for (j = 0; j < all_tests_cnt; j++)
                if (strcmp(argv[i], all_tests[j].name) == 0)
                    run_test(&all_tests[j]);
    }

    return 0;
}
