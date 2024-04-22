#include "algo/lamportAlgorithm.h"
#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>

void processUserInput(LamportAlgorithm* algorithm){
    std::cout << "CHOOSE ONE: EXIT, REQUEST, STATUS\n";
    std::string input;
    while(true){
        std::cout << ">";
        std::cin >> input;
        if(input == "REQUEST"){
            algorithm->requestCriticalSection();
        }
        if(input == "EXIT"){
            return;
        }
        if(input == "STATUS"){
            algorithm->printConfiguration();
        }
    }
}

int main(int argc, char* argv[]) {
    int port = 0;
    int processId = 0;
    std::string filename;

    // Input check for port and processId
    if (argc < 5) {
         std::cerr << "Error: Insufficient arguments. Usage: " << argv[0] << " -p <PORT NO> -i <PROCESS ID> -f <CONFIG FILE>\n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if ((arg == "-p") && i + 1 < argc) {
            port = std::stoi(argv[++i]); // Convert string to int
        } else if ((arg == "-i") && i + 1 < argc) {
            processId = std::stoi(argv[++i]); // Convert string to int
        } else if ((arg == "-f") && i + 1 < argc) {
            filename = argv[++i]; // Convert string to int
        } else {
            std::cerr << "Error: Invalid arguments. Usage: " << argv[0] << " -p <PORT NO> -i <PROCESS ID> -f <CONFIG FILE>\n";
            return 1;
        }
    }
    
    LamportAlgorithm* algorithm = new LamportAlgorithm(processId, port);

    // Handle config
    std::ifstream file(filename);
     if (!file.is_open()) {
        std::cerr << "Error: Failed to open config file: " << filename << std::endl;
        delete algorithm;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int nodeId, nodePort;
        if (iss >> nodePort >> nodeId) {
            algorithm->addNode(nodeId, "127.0.0.1", nodePort);
        }
    }

    // Initialize the listener thread
    std::thread listenerThread(&LamportAlgorithm::startListening, algorithm);
    if (!listenerThread.joinable()) {
        std::cerr << "Error: Failed to create listener thread" << std::endl;
        delete algorithm;
        return 1;
    }
    // Initialize the queue handler thread
    std::thread queueHandlerThread(&LamportAlgorithm::handleQueue, algorithm);
     if (!queueHandlerThread.joinable()) {
        std::cerr << "Error: Failed to create queue handler thread" << std::endl;
        delete algorithm;
        return 1;
    }
    // Initialize the input handler thread
    std::thread inputHandlerThread(processUserInput, algorithm);
    if (!inputHandlerThread.joinable()) {
        std::cerr << "Error: Failed to create input handler thread" << std::endl;
        delete algorithm;
        return 1;
    }
    // Wait for the listener thread to finish
    listenerThread.join();
    // Wait for the queue handler thread to finish
    queueHandlerThread.join();
    // Wait for the input handler thread to finish
    inputHandlerThread.join();

    return 0;
}
