#include <iostream>
#include <cstring> // for strerror
#include <cerrno> // for errno
#include "lamportAlgorithm.h"



// Error handling function to print error messages with descriptive context
void LamportAlgorithm::handleError(const std::string& message) {
    std::cerr << "Error: " << message << ": " << strerror(errno) << std::endl;
}

// Function to handle socket operations with error checking
int LamportAlgorithm::safeSocket(int domain, int type, int protocol) {
    int sock = socket(domain, type, protocol);
    if (sock < 0) {
        handleError("Socket creation failed");
    }
    return sock;
}

// Function to handle listen operations with error checking
int LamportAlgorithm::safeListen(int sockfd, int backlog) {
    int status = listen(sockfd, backlog);
    if (status < 0) {
        handleError("Listen failed");
    }
    return status;
}

// Function to handle close operations with error checking
int LamportAlgorithm::safeClose(int sockfd) {
    int status = close(sockfd);
    if (status < 0) {
        handleError("Close failed");
    }
    return status;
}

// Function to handle getaddrinfo operations with error checking
int LamportAlgorithm::safeGetAddrInfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res) {
    int status = getaddrinfo(node, service, hints, res);
    if (status != 0) {
        handleError("Getaddrinfo failed");
    }
    return status;
}




// Function to handle send operations with error checking

LamportAlgorithm::LamportAlgorithm(int id, int lport){
    logicalClock = 0;
    processIdentifier = id;
    listenPort = lport;
}

LamportAlgorithm::~LamportAlgorithm(){

}

int LamportAlgorithm::safeConnect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    int status = connect(sockfd, addr, addrlen);
    if (status < 0) {
        handleError("Connection failed");
    }
    return status;
}




int LamportAlgorithm::safeAccept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    int clientSock = accept(sockfd, addr, addrlen);
    if (clientSock < 0) {
        handleError("Accept failed");
    }
    return clientSock;
}

void LamportAlgorithm::addNode(int id, std::string ip, int port){
    struct sockaddr_in node;
    node.sin_family = AF_INET;
    node.sin_port = htons(port);
    node.sin_addr.s_addr = inet_addr(ip.c_str());
    nodeList[id] = node;
}

// Function to handle receive operations with error checking
int LamportAlgorithm::safeRecv(int sockfd, void* buf, size_t len, int flags) {
    int bytesReceived = recv(sockfd, buf, len, flags);
    if (bytesReceived < 0) {
        handleError("Receive failed");
    }
    return bytesReceived;
}


void LamportAlgorithm::initiateRequest(){
    // Increment clock

    std::unique_lock<std::mutex> lock(clockMutex);
    logicalClock++;
    lock.unlock();

    broadcastSignal(REQUEST_SIGNAL);
    requestQueue.push({logicalClock, processIdentifier});
}

void LamportAlgorithm::receiveData(){
    // Setup server on listenport
    int servsock = socket(AF_INET, SOCK_STREAM, 0);
    if(servsock < 0){
        perror("Error in Socket creation");
        return;
    }

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(listenPort);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(servsock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0){
        perror("Bind failed");
        return;
    }

    SynchronizationData data;

    while(true){
        // Listen for incoming connections
        listen(servsock, 5);

        // Accept the connection
        int sock = accept(servsock, NULL, NULL);
        if(sock < 0){
            perror("Accept failed");
            return;
        }

        // Receive the data
        if(recv(sock, &data, sizeof(data), 0) < 0){
            perror("Receive failed");
            return;
        }

        // Close the socket
        close(sock);

        // Handle the received data
        handleReceivedData(data);
    }
}


int LamportAlgorithm::safeSend(int sockfd, const void* buf, size_t len, int flags) {
    int bytesSent = send(sockfd, buf, len, flags);
    if (bytesSent < 0) {
        handleError("Send failed");
    }
    return bytesSent;
} 

int LamportAlgorithm::sendSignal(UnicastSignal sig, int sysId){
    // Fetch sockAddr from nodeList
    struct sockaddr_in node = nodeList[sysId];

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("Error in Socket creation");
        return -1;
    }

    // Connect to the node
    if(connect(sock, (struct sockaddr*)&node, sizeof(node)) < 0){
        perror("Error in Connection");
        return -1;
    }

    SynchronizationData data;

    // Lock the clock
    std::unique_lock<std::mutex> lock(clockMutex);

    // Get the clock
    data.timestamp = logicalClock;

    // Set the senderId
    data.senderId = processIdentifier;

    // Set the message type
    data.messageType = sig;

    // Unlock the clock
    lock.unlock();

    // Send the data
    if(send(sock, &data, sizeof(data), 0) < 0){
        perror("Send failed");
        return -1;
    }

    // Close the socket
    close(sock);

    return 0;
}

int LamportAlgorithm::safePthreadCondBroadcast(pthread_cond_t* cond) {
    int status = pthread_cond_broadcast(cond);
    if (status != 0) {
        handleError("Condition variable broadcast failed");
    }
    return status;
}

int LamportAlgorithm::broadcastSignal(UnicastSignal sig){
    auto it = nodeList.begin();
    while (it != nodeList.end()) {
        if (it->first != processIdentifier) {
            sendSignal(sig, it->first);
        }
        it++;
    }
    return 0;
}

void LamportAlgorithm::handleReceivedData(SynchronizationData data){
    // Lock the clock
    std::unique_lock<std::mutex> lock(clockMutex);

    // Update the clock
    logicalClock = std::max(logicalClock, data.timestamp) + 1;

    // Unlock the clock
    lock.unlock();

    switch(data.messageType){
        case REQUEST_SIGNAL:
            std::cout << logicalClock << "(clock)\nrequest received from sender with id:\n" << data.senderId << std::endl;
            // Add the request to the queue
            requestQueue.push({data.timestamp, data.senderId});

            // If top of requestQueue is not this process, reply
            if(requestQueue.top().second != processIdentifier){
                sendSignal(REPLY_SIGNAL, data.senderId);
            }

            break;

        case REPLY_SIGNAL:
            // Add reply to the replyMap
            std::cout << logicalClock << "(clock)\nreply received from sender with id:\n " << data.senderId << std::endl;
            replyMap.insert(data.senderId);
            break;

        case RELEASE_SIGNAL:
            // Do something
            std::cout << logicalClock << "(clock)\nrelease received from sender with id:\n" << data.senderId << std::endl;
            if(data.senderId == requestQueue.top().second)
                requestQueue.pop();
            else{
                perror("Release is invalid");
                exit(1);
            }
            break;
    }
}

// Function to handle bind operations with error checking
int LamportAlgorithm::safeBind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    int status = bind(sockfd, addr, addrlen);
    if (status < 0) {
        handleError("Bind failed");
    }
    return status;
}

void LamportAlgorithm::handleRequestQueue(){
    while(true){
        if(!requestQueue.empty()){

            // If the top request is from this process
            if(requestQueue.top().second == processIdentifier){

                // Check if all replies are received
                if(replyMap.size() == nodeList.size() - 1){

                    //increment clock
                    std::unique_lock<std::mutex> lock(clockMutex);
                    logicalClock++;
                    lock.unlock();

                    std::cout << " Critial Section entered by (clock):\n" << logicalClock << std::endl;
                    // Enter the critical section
                    std::this_thread::sleep_for(std::chrono::seconds(10));

                    // Broadcast release signal
                    broadcastSignal(RELEASE_SIGNAL);

                    // Clear the replyMap
                    replyMap.clear();

                    //pop from the pqueue
                    requestQueue.pop();

                    std::cout << "Critical Section exited by (clock):\n" << logicalClock << std::endl;
                }
            }
        }
    }
}

int LamportAlgorithm::safePthreadCreate(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
    int status = pthread_create(thread, attr, start_routine, arg);
    if (status != 0) {
        handleError("Pthread create failed");
    }
    return status;
}

// Function to handle pthread_join operations with error checking
int LamportAlgorithm::safePthreadJoin(pthread_t thread, void** retval) {
    int status = pthread_join(thread, retval);
    if (status != 0) {
        handleError("Pthread join failed");
    }
    return status;
}

void LamportAlgorithm::printConfiguration(){


    std::cout << "Process_ID: " << processIdentifier;
    std::cout << " Listen_Port: " << listenPort;
    std::cout << " Clock: " << logicalClock << std::endl;

    std::cout << "Node List: " << std::endl;

    for(auto it = nodeList.begin(); it != nodeList.end(); it++){
        std::cout << "Node_Id=> " << it->first << " Port=> " << ntohs(it->second.sin_port) << " Host=> " << inet_ntoa(it->second.sin_addr) << std::endl;
    }

    std::cout << "Request Queue: " << std::endl;

    //print the request queue
    std::priority_queue<std::pair<int, int>,std::vector<std::pair<int,int>>,std::greater<std::pair<int,int>>> temp = requestQueue;
    while(!temp.empty()){
        std::cout << "Node_ID=> " << temp.top().second << " Timestamp=> " << temp.top().first << std::endl;
        temp.pop();
    }

    std::cout << std::endl;

    std::cout << "Reply Map: " << std::endl;

    //Print reply map
    for(auto it = replyMap.begin(); it != replyMap.end(); it++){
        std::cout << "ID: " << *it << std::endl;
    }

    std::cout << std::endl;
}



// Function to handle pthread_mutex_lock operations with error checking
int LamportAlgorithm::safePthreadMutexLock(pthread_mutex_t* mutex) {
    int status = pthread_mutex_lock(mutex);
    if (status != 0) {
        handleError("Mutex lock failed");
    }
    return status;
}

// Function to handle pthread_mutex_unlock operations with error checking
int LamportAlgorithm::safePthreadMutexUnlock(pthread_mutex_t* mutex) {
    int status = pthread_mutex_unlock(mutex);
    if (status != 0) {
        handleError("Mutex unlock failed");
}
return status;
}



