#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <vector>
#include <iostream>
#include <mutex>
#include <condition_variable>

class ResourceManager {
private:
    int numProcesses;
    int numResources;
    std::vector<int> available;
    std::vector<std::vector<int>> max_matrix;
    std::vector<std::vector<int>> allocation;
    std::vector<std::vector<int>> need;
    std::mutex mtx;
    std::condition_variable cv;

    void calculateNeed() {
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                need[i][j] = max_matrix[i][j] - allocation[i][j];
            }
        }
    }

    bool checkSafeState() {
        std::vector<int> work = available;
        std::vector<bool> finish(numProcesses, false);

        int count = 0;
        while (count < numProcesses) {
            bool found = false;
            for (int i = 0; i < numProcesses; i++) {
                if (!finish[i]) {
                    bool canAllocate = true;
                    for (int j = 0; j < numResources; j++) {
                        if (need[i][j] > work[j]) {
                            canAllocate = false;
                            break;
                        }
                    }
                    if (canAllocate) {
                        for (int j = 0; j < numResources; j++) {
                            work[j] += allocation[i][j];
                        }
                        finish[i] = true;
                        found = true;
                        count++;
                    }
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

public:
    ResourceManager(int processes, int resources, std::vector<int> avail,
                    std::vector<std::vector<int>> max_mat)
        : numProcesses(processes), numResources(resources), available(avail),
          max_matrix(max_mat) {
        
        allocation.resize(numProcesses, std::vector<int>(numResources, 0));
        need.resize(numProcesses, std::vector<int>(numResources, 0));
        calculateNeed();
    }

    bool isSafeState() {
        std::lock_guard<std::mutex> lock(mtx);
        std::vector<int> work = available;
        std::vector<bool> finish(numProcesses, false);
        std::vector<int> safeSequence;

        int count = 0;
        while (count < numProcesses) {
            bool found = false;
            for (int i = 0; i < numProcesses; i++) {
                if (!finish[i]) {
                    bool canAllocate = true;
                    for (int j = 0; j < numResources; j++) {
                        if (need[i][j] > work[j]) {
                            canAllocate = false;
                            break;
                        }
                    }
                    if (canAllocate) {
                        for (int j = 0; j < numResources; j++) {
                            work[j] += allocation[i][j];
                        }
                        finish[i] = true;
                        safeSequence.push_back(i);
                        found = true;
                        count++;
                    }
                }
            }
            if (!found) {
                return false;
            }
        }

        std::cout << "Safe Sequence: ";
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << safeSequence[i];
            if (i != numProcesses - 1) std::cout << " -> ";
        }
        std::cout << std::endl;
        return true;
    }

    bool request(int processID, std::vector<int> requestVec) {
        std::unique_lock<std::mutex> lock(mtx);

        for (int j = 0; j < numResources; j++) {
            if (requestVec[j] > need[processID][j]) {
                std::cout << "[P" << processID << "] Error: exceeded max claim" << std::endl;
                return false;
            }
        }

        while (true) {
            bool resourcesAvailable = true;
            for (int j = 0; j < numResources; j++) {
                if (requestVec[j] > available[j]) {
                    resourcesAvailable = false;
                    break;
                }
            }

            if (!resourcesAvailable) {
                std::cout << "[P" << processID << "] Waiting for resources..." << std::endl;
                cv.wait(lock);
                continue;
            }

            for (int j = 0; j < numResources; j++) {
                available[j] -= requestVec[j];
                allocation[processID][j] += requestVec[j];
                need[processID][j] -= requestVec[j];
            }

            if (checkSafeState()) {
                std::cout << "[P" << processID << "] Request granted" << std::endl;
                return true;
            } else {
                for (int j = 0; j < numResources; j++) {
                    available[j] += requestVec[j];
                    allocation[processID][j] -= requestVec[j];
                    need[processID][j] += requestVec[j];
                }
                std::cout << "[P" << processID << "] Request denied (unsafe), waiting..." << std::endl;
                cv.wait(lock);
            }
        }
    }

    void release(int processID, std::vector<int> releaseVec) {
        std::lock_guard<std::mutex> lock(mtx);
        for (int j = 0; j < numResources; j++) {
            available[j] += releaseVec[j];
            allocation[processID][j] -= releaseVec[j];
            need[processID][j] += releaseVec[j];
        }
        std::cout << "[P" << processID << "] Released resources" << std::endl;
        cv.notify_all();
    }

    void printState() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "\n=== Current System State ===" << std::endl;
        std::cout << "Available: ";
        for (int j = 0; j < numResources; j++) {
            std::cout << available[j] << " ";
        }
        std::cout << "\n\nAllocation Matrix:" << std::endl;
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                std::cout << allocation[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "\nNeed Matrix:" << std::endl;
        for (int i = 0; i < numProcesses; i++) {
            std::cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                std::cout << need[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "============================\n" << std::endl;
    }

    int getNumResources() { return numResources; }
    std::vector<int> getNeed(int processID) { 
        std::lock_guard<std::mutex> lock(mtx);
        return need[processID]; 
    }
};

#endif
