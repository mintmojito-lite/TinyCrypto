#include "include/fe.hpp"
#include <iostream>
#include <cstdio>
using namespace tinycrypto;
int main() {
    Fe a(2,0,0,0,0,0,0,0,0,0);
    Fe inv = a.invert();
    Fe one = a * inv;
    uint8_t out[32];
    one.to_bytes(out);
    printf("2 * inv(2) = ");
    for(int i=0; i<32; i++) printf("%02x", out[i]);
    printf("\n");

    Fe nine(9,0,0,0,0,0,0,0,0,0);
    Fe eightyone = nine.square();
    eightyone.to_bytes(out);
    printf("9^2 = ");
    for(int i=0; i<32; i++) printf("%02x", out[i]);
    printf("\n");
}
