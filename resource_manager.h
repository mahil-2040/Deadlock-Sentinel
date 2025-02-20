#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <vector>
#include <iostream>

class ResourceManager {
private:
    int numProcesses;
    int numResources;
    std::vector<int> available;
    std::vector<std::vector<int>> max_matrix;
    std::vector<std::vector<int>> allocation;
    std::vector<std::vector<int>> need;

    void calculateNeed() {
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                need[i][j] = max_matrix[i][j] - allocation[i][j];
            }
        }
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
        for (int j = 0; j < numResources; j++) {
            if (requestVec[j] > need[processID][j]) {
                std::cout << "Error: Process " << processID << " exceeded max claim" << std::endl;
                return false;
            }
        }

        for (int j = 0; j < numResources; j++) {
            if (requestVec[j] > available[j]) {
                std::cout << "Process " << processID << " must wait (resources not available)" << std::endl;
                return false;
            }
        }

        for (int j = 0; j < numResources; j++) {
            available[j] -= requestVec[j];
            allocation[processID][j] += requestVec[j];
            need[processID][j] -= requestVec[j];
        }

        if (isSafeState()) {
            std::cout << "Request granted for Process " << processID << std::endl;
            return true;
        } else {
            for (int j = 0; j < numResources; j++) {
                available[j] += requestVec[j];
                allocation[processID][j] -= requestVec[j];
                need[processID][j] += requestVec[j];
            }
            std::cout << "Request denied (would lead to unsafe state) for Process " << processID << std::endl;
            return false;
        }
    }

    void release(int processID, std::vector<int> releaseVec) {
        for (int j = 0; j < numResources; j++) {
            available[j] += releaseVec[j];
            allocation[processID][j] -= releaseVec[j];
            need[processID][j] += releaseVec[j];
        }
        std::cout << "Process " << processID << " released resources" << std::endl;
    }

    void printState() {
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
};

#endif
