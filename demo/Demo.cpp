#include <cstddef>
#include <cstdio>
#include <iostream>
#include "Xorstr.hpp"

__declspec(noinline) int add(int x, int y) {
    return x + y;
}

int main() {
    auto answer = add(4, 2);
    printf(XOR("answer: %i\n"), answer);

    std::string c;
    std::cin >> c;

    if (c[0] == 'X') {
        printf(XOR("yes\n"));
    } else {
        printf(XOR("no\n"));
    }
}