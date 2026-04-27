#include "tests/tests.hpp"
#include <cstdio>
using namespace std;
int main() {
    bool slave_tests = Tests::test_slave();
    bool master_tests = Tests::test_master();
}