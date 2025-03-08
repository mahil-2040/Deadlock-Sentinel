#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include "resource_manager.h"

using namespace std;
ResourceManager* rm;

void processThread(int processID) {
    random_device rd;
    mt19937 gen(rd());
    
    for (int iteration = 0; iteration < 3; iteration++) {
        this_thread::sleep_for(chrono::milliseconds(100 + (processID * 50)));
        
        vector<int> need = rm->getNeed(processID);
        vector<int> request(rm->getNumResources());
        
        bool hasRequest = false;
        for (int j = 0; j < rm->getNumResources(); j++) {
            uniform_int_distribution<> dis(0, min(need[j], 2));
            request[j] = dis(gen);
            if (request[j] > 0) hasRequest = true;
        }
        
        if (hasRequest) {
            cout << "[P" << processID << "] Requesting: ";
            for (int r : request) cout << r << " ";
            cout << endl;
            
            rm->request(processID, request);
            
            this_thread::sleep_for(chrono::milliseconds(200 + (processID * 30)));
            
            vector<int> toRelease(rm->getNumResources());
            for (int j = 0; j < rm->getNumResources(); j++) {
                uniform_int_distribution<> dis(0, request[j]);
                toRelease[j] = dis(gen);
            }
            
            bool hasRelease = false;
            for (int r : toRelease) if (r > 0) hasRelease = true;
            
            if (hasRelease) {
                cout << "[P" << processID << "] Releasing: ";
                for (int r : toRelease) cout << r << " ";
                cout << endl;
                rm->release(processID, toRelease);
            }
        }
    }
    cout << "[P" << processID << "] Finished" << endl;
}

int main() {
    cout << "=== SafeState: Multi-Threaded Simulation ===" << endl;
    cout << "Running 5 concurrent processes..." << endl << endl;

    int numProcesses = 5;
    int numResources = 3;

    vector<int> available = {10, 5, 7};

    vector<vector<int>> max_matrix = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    rm = new ResourceManager(numProcesses, numResources, available, max_matrix);
    rm->printState();

    vector<thread> threads;
    for (int i = 0; i < numProcesses; i++) {
        threads.push_back(thread(processThread, i));
    }

    for (auto& t : threads) {
        t.join();
    }

    cout << "\n=== Simulation Complete ===" << endl;
    rm->printState();
    rm->dumpState();
    rm->generateDotFile("allocation_graph.dot");

    cout << "\n=== Admin Feature: Force Reclaim Demo ===" << endl;
    int hoggingProcess = rm->getProcessWithMostResources();
    if (hoggingProcess >= 0) {
        cout << "Process P" << hoggingProcess << " is holding the most resources." << endl;
        rm->forceReclaim(hoggingProcess);
        rm->printState();
    }

    delete rm;
    return 0;
}
