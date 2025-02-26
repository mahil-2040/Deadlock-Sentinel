#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include "resource_manager.h"

ResourceManager* rm;

void processThread(int processID) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int iteration = 0; iteration < 3; iteration++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + (processID * 50)));
        
        std::vector<int> need = rm->getNeed(processID);
        std::vector<int> request(rm->getNumResources());
        
        bool hasRequest = false;
        for (int j = 0; j < rm->getNumResources(); j++) {
            std::uniform_int_distribution<> dis(0, std::min(need[j], 2));
            request[j] = dis(gen);
            if (request[j] > 0) hasRequest = true;
        }
        
        if (hasRequest) {
            std::cout << "[P" << processID << "] Requesting: ";
            for (int r : request) std::cout << r << " ";
            std::cout << std::endl;
            
            rm->request(processID, request);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200 + (processID * 30)));
            
            std::vector<int> toRelease(rm->getNumResources());
            for (int j = 0; j < rm->getNumResources(); j++) {
                std::uniform_int_distribution<> dis(0, request[j]);
                toRelease[j] = dis(gen);
            }
            
            bool hasRelease = false;
            for (int r : toRelease) if (r > 0) hasRelease = true;
            
            if (hasRelease) {
                std::cout << "[P" << processID << "] Releasing: ";
                for (int r : toRelease) std::cout << r << " ";
                std::cout << std::endl;
                rm->release(processID, toRelease);
            }
        }
    }
    std::cout << "[P" << processID << "] Finished" << std::endl;
}

int main() {
    std::cout << "=== SafeState: Multi-Threaded Simulation ===" << std::endl;
    std::cout << "Running 5 concurrent processes..." << std::endl << std::endl;

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

    rm = new ResourceManager(numProcesses, numResources, available, max_matrix);
    rm->printState();

    std::vector<std::thread> threads;
    for (int i = 0; i < numProcesses; i++) {
        threads.push_back(std::thread(processThread, i));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\n=== Simulation Complete ===" << std::endl;
    rm->printState();

    delete rm;
    return 0;
}
