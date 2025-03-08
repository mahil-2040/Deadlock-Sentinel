#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

class ResourceManager {
private:
    int numProcesses;
    int numResources;
    vector<int> available;
    vector<vector<int>> max_matrix;
    vector<vector<int>> allocation;
    vector<vector<int>> need;
    mutex mtx;
    condition_variable cv;

    void calculateNeed() {
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                need[i][j] = max_matrix[i][j] - allocation[i][j];
            }
        }
    }

    bool checkSafeState() {
        vector<int> work = available;
        vector<bool> finish(numProcesses, false);

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
    ResourceManager(int processes, int resources, vector<int> avail,
                    vector<vector<int>> max_mat)
        : numProcesses(processes), numResources(resources), available(avail),
          max_matrix(max_mat) {
        
        allocation.resize(numProcesses, vector<int>(numResources, 0));
        need.resize(numProcesses, vector<int>(numResources, 0));
        calculateNeed();
    }

    bool isSafeState() {
        lock_guard<mutex> lock(mtx);
        vector<int> work = available;
        vector<bool> finish(numProcesses, false);
        vector<int> safeSequence;

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

        cout << "Safe Sequence: ";
        for (int i = 0; i < numProcesses; i++) {
            cout << "P" << safeSequence[i];
            if (i != numProcesses - 1) cout << " -> ";
        }
        cout << endl;
        return true;
    }

    bool request(int processID, vector<int> requestVec) {
        unique_lock<mutex> lock(mtx);
        int retries = 0;
        const int maxRetries = 5;

        for (int j = 0; j < numResources; j++) {
            if (requestVec[j] > need[processID][j]) {
                cout << "[P" << processID << "] Error: exceeded max claim" << endl;
                return false;
            }
        }

        while (retries < maxRetries) {
            bool resourcesAvailable = true;
            for (int j = 0; j < numResources; j++) {
                if (requestVec[j] > available[j]) {
                    resourcesAvailable = false;
                    break;
                }
            }

            if (!resourcesAvailable) {
                cout << "[P" << processID << "] Waiting for resources..." << endl;
                cv.wait_for(lock, chrono::milliseconds(500));
                retries++;
                continue;
            }

            for (int j = 0; j < numResources; j++) {
                available[j] -= requestVec[j];
                allocation[processID][j] += requestVec[j];
                need[processID][j] -= requestVec[j];
            }

            if (checkSafeState()) {
                cout << "[P" << processID << "] Request granted" << endl;
                return true;
            } else {
                for (int j = 0; j < numResources; j++) {
                    available[j] += requestVec[j];
                    allocation[processID][j] -= requestVec[j];
                    need[processID][j] += requestVec[j];
                }
                cout << "[P" << processID << "] Request denied (unsafe), waiting..." << endl;
                cv.wait_for(lock, chrono::milliseconds(500));
                retries++;
            }
        }
        cout << "[P" << processID << "] Request timed out" << endl;
        return false;
    }

    void release(int processID, vector<int> releaseVec) {
        lock_guard<mutex> lock(mtx);
        for (int j = 0; j < numResources; j++) {
            available[j] += releaseVec[j];
            allocation[processID][j] -= releaseVec[j];
            need[processID][j] += releaseVec[j];
        }
        cout << "[P" << processID << "] Released resources" << endl;
        cv.notify_all();
    }

    void printState() {
        lock_guard<mutex> lock(mtx);
        cout << "\n=== Current System State ===" << endl;
        cout << "Available: ";
        for (int j = 0; j < numResources; j++) {
            cout << available[j] << " ";
        }
        cout << "\n\nAllocation Matrix:" << endl;
        for (int i = 0; i < numProcesses; i++) {
            cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                cout << allocation[i][j] << " ";
            }
            cout << endl;
        }
        cout << "\nNeed Matrix:" << endl;
        for (int i = 0; i < numProcesses; i++) {
            cout << "P" << i << ": ";
            for (int j = 0; j < numResources; j++) {
                cout << need[i][j] << " ";
            }
            cout << endl;
        }
        cout << "============================\n" << endl;
    }

    void dumpState() {
        lock_guard<mutex> lock(mtx);
        cout << "\n=== Resource Allocation Graph ===" << endl;
        
        for (int i = 0; i < numProcesses; i++) {
            cout << "P" << i << " holds: ";
            for (int j = 0; j < numResources; j++) {
                if (allocation[i][j] > 0) {
                    cout << "R" << j << "(" << allocation[i][j] << ") ";
                }
            }
            cout << "| needs: ";
            for (int j = 0; j < numResources; j++) {
                if (need[i][j] > 0) {
                    cout << "R" << j << "(" << need[i][j] << ") ";
                }
            }
            cout << endl;
        }
        
        cout << "\nResource Pool: ";
        for (int j = 0; j < numResources; j++) {
            cout << "R" << j << "=" << available[j] << " ";
        }
        cout << "\n================================\n" << endl;
    }

    void generateDotFile(const string& filename) {
        lock_guard<mutex> lock(mtx);
        ofstream file(filename);
        
        file << "digraph RAG {" << endl;
        file << "    rankdir=LR;" << endl;
        file << "    node [shape=circle]; ";
        for (int i = 0; i < numProcesses; i++) {
            file << "P" << i << " ";
        }
        file << ";" << endl;
        
        file << "    node [shape=box]; ";
        for (int j = 0; j < numResources; j++) {
            file << "R" << j << " ";
        }
        file << ";" << endl;
        
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                if (allocation[i][j] > 0) {
                    file << "    R" << j << " -> P" << i;
                    file << " [label=\"" << allocation[i][j] << "\"];" << endl;
                }
            }
        }
        
        for (int i = 0; i < numProcesses; i++) {
            for (int j = 0; j < numResources; j++) {
                if (need[i][j] > 0) {
                    file << "    P" << i << " -> R" << j;
                    file << " [label=\"" << need[i][j] << "\", style=dashed];" << endl;
                }
            }
        }
        
        file << "}" << endl;
        file.close();
        cout << "Generated " << filename << " (use 'dot -Tpng " << filename << " -o graph.png' to visualize)" << endl;
    }

    int getNumResources() { return numResources; }
    vector<int> getNeed(int processID) { 
        lock_guard<mutex> lock(mtx);
        return need[processID]; 
    }

    void forceReclaim(int processID) {
        lock_guard<mutex> lock(mtx);
        cout << "\n!!! FORCE RECLAIM: Terminating Process " << processID << " !!!" << endl;
        
        cout << "Reclaiming resources: ";
        for (int j = 0; j < numResources; j++) {
            if (allocation[processID][j] > 0) {
                cout << "R" << j << "(" << allocation[processID][j] << ") ";
                available[j] += allocation[processID][j];
            }
            need[processID][j] += allocation[processID][j];
            allocation[processID][j] = 0;
        }
        cout << endl;
        
        cout << "Process " << processID << " killed and resources returned to pool." << endl;
        cv.notify_all();
    }

    int getProcessWithMostResources() {
        lock_guard<mutex> lock(mtx);
        int maxProcess = -1;
        int maxResources = 0;
        
        for (int i = 0; i < numProcesses; i++) {
            int total = 0;
            for (int j = 0; j < numResources; j++) {
                total += allocation[i][j];
            }
            if (total > maxResources) {
                maxResources = total;
                maxProcess = i;
            }
        }
        return maxProcess;
    }
};

#endif
