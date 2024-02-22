#include <stdio.h>

void foo() {
    void *ret_addr = __builtin_return_address(0);
    printf("Return address: %p\n", ret_addr);
}

int main() {
    foo();
    return 0;
}
