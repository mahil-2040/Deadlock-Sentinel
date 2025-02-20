#include <iostream>
#include <vector>
#include "resource_manager.h"

int main() {
    std::cout << "=== SafeState: Banker's Algorithm Demo ===" << std::endl;

    int numProcesses = 5;
    int numResources = 3;

    std::vector<int> available = {10, 5, 7};

    std::vector<std::vector<int>> max_matrix = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    ResourceManager rm(numProcesses, numResources, available, max_matrix);
    rm.printState();

    std::cout << "\n--- P1 requests (1, 0, 2) ---" << std::endl;
    rm.request(1, {1, 0, 2});
    rm.printState();

    std::cout << "\n--- P3 requests (2, 1, 1) ---" << std::endl;
    rm.request(3, {2, 1, 1});
    rm.printState();

    std::cout << "\n--- P0 requests (0, 1, 0) ---" << std::endl;
    rm.request(0, {0, 1, 0});
    rm.printState();

    std::cout << "\n--- P2 requests (3, 0, 2) ---" << std::endl;
    rm.request(2, {3, 0, 2});
    rm.printState();

    std::cout << "\n--- P1 releases (1, 0, 2) ---" << std::endl;
    rm.release(1, {1, 0, 2});
    rm.printState();

    std::cout << "\n--- Testing large request from P0 ---" << std::endl;
    rm.request(0, {7, 4, 3});

    return 0;
}
