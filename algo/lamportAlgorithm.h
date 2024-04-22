#ifndef LAMPORT_ALGORITHM_H
#define LAMPORT_ALGORITHM_H

#include <set>
#include <map>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <condition_variable>
#include <atomic>
#include <unistd.h>
#include <iostream>

enum UnicastSignal {
    REQUEST_SIGNAL, //SIGNALS MEANT TO REQUEST THE CRITICAL SECTION
    REPLY_SIGNAL, //SIGNALS MEANT TO REPLY TO A REQUEST
    RELEASE_SIGNAL, //SIGNALS MEANT TO RELEASE THE CRITICAL SECTION
};

class SynchronizationData {
public:
    int timestamp;
    int senderId;
    UnicastSignal messageType;
};

class LamportAlgorithm {
private:
    int processIdentifier; // Unique ID for each process
    int listenPort; // Port for listening to incoming messages
    int logicalClock; // Atomic variable for logical clock
    std::mutex clockMutex; // Mutex for protecting logical clock updates
    std::priority_queue<std::pair<int, int>,std::vector<std::pair<int,int>>,std::greater<std::pair<int,int>>> requestQueue; // Queue for storing requests
    std::set<int> replyMap; // Map for storing reply status
    std::map<int, struct sockaddr_in> nodeList; // Map for storing node information
    
public:
    LamportAlgorithm(int id, int lport);
    ~LamportAlgorithm();

    // BASE FUNCTIONALITIES

    // ADD SYSTEM TO THE NODE LIST
    void addNode(int id, std::string ip, int port);

   
    
    // SEND REQUEST SIGNAL
    void initiateRequest();
    
    // RECIEVE DATA FROM THE PORT, ISKO THREAD PE CHADHAYENGE
    void receiveData();

    // HANDLE RECIEVED DATA
    void handleReceivedData(SynchronizationData data);


    // MAINTAIN THE REQUEST QUEUE, ISKO BHI THREAD PE CHADHAYENGE
    void handleRequestQueue();
    
     // FIND THE PORT AND SEND THE DATA
    int sendSignal(UnicastSignal signal, int sysId);

    // SEND DATA TO ALL NODES
    int broadcastSignal(UnicastSignal signal);

    // PRINT CONFIG
    void printConfiguration();
    private:
    // Error handling function
    void handleError(const std::string& message);

    // Wrapper functions with error handling for socket operations
    int safeConnect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
    int safeBind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
    int safeListen(int sockfd, int backlog);
    int safeAccept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
    int safeRecv(int sockfd, void* buf, size_t len, int flags);
    int safeSend(int sockfd, const void* buf, size_t len, int flags);

    // Wrapper functions with error handling for pthread operations
    int safePthreadCreate(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
    int safePthreadJoin(pthread_t thread, void** retval);
    int safePthreadMutexLock(pthread_mutex_t* mutex);
    int safePthreadMutexUnlock(pthread_mutex_t* mutex);
};


#endif