

#define CHECK(label, cond, pass, fail) \
        if (cond) { printf("PASS - %s\n", label); (*pass)++; } \
        else      { printf("FAIL - %s\n", label); (*fail)++; }
namespace Tests {
    int test_master();
    int test_slave();
}