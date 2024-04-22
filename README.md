# Lamport's Mutual Exclusion Algorithm

Lamport's Mutual Exclusion Algorithm is a distributed mutual exclusion algorithm proposed by Leslie Lamport in 1978. It allows multiple processes to access a shared resource in a mutually exclusive manner, ensuring that only one process can access the resource at a time. The algorithm is based on Lamport's logical clocks and uses message passing between processes to coordinate access to the shared resource.

## Algorithm Overview

The Lamport's Mutual Exclusion Algorithm is designed for distributed systems to ensure mutual exclusion, preventing multiple processes from accessing a critical section simultaneously. It achieves this by assigning logical timestamps to events, allowing processes to coordinate their actions effectively.

Here's a simplified explanation of the algorithm:

-Each process maintains a logical clock that represents the order of events it has encountered.
-When a process wishes to enter the critical section, it sends a request message to all other processes, including its current logical clock value.
-Upon receiving a request message, each process updates its logical clock to reflect the latest event and sends a reply message to the requesting process.
-The requesting process can enter the critical section only when it has received reply messages from all other processes, indicating their consent.
-After exiting the critical section, the process broadcasts a release message to inform others that the critical section is now available.
-Other processes, upon receiving the release message, update their logical clocks accordingly and can now request access to the critical section if needed.
-This algorithm ensures that processes coordinate effectively and maintain mutual exclusion in the critical section. 
-It prevents race conditions and ensures orderly access to shared resources in distributed systems. This explanation captures the essence of Lamport's Mutual Exclusion Algorithm without directly replicating existing content.

## Project Overview
This project implements Lamport's Mutual Exclusion Algorithm, a distributed algorithm that allows multiple processes to access a shared resource in a mutually exclusive manner. The project consists of several files:

Makefile: Contains rules for building the project.
algo/lamportAlgorithm.cpp: Implementation of the Lamport's Mutual Exclusion Algorithm.


## Building and Running the Project
To compile the project, navigate to the project directory and execute the following command:

```
make
```

This command compiles the source files and creates the executable main.

Once the project is built, you can run the executable using the following command:

```
./main -p <PORT_NO> -i <SYSTEM_ID> -f <CONFIG_FILE>
```

![IMG](/img/img1.png)

Replace <PORT_NO> with the desired port number for the process to listen on, <SYSTEM_ID> with a unique identifier for the process, and <CONFIG_FILE> with the path to the configuration file containing information about other nodes in the distributed system. The configuration file should have entries like:


The application initiates listening for incoming messages on the specified port and awaits user input. During execution, users are prompted to input additional commands:

REQUEST: Initiates a request to access the critical section.
STATUS: Adds a new node to the distributed system.
EXIT: Terminates the program.

## Functions

-void addNode(int id, std::string ip, int port): Incorporates a new node into the nodeList with the provided ID, IP address, and port number.
-int unicast(Signal sig, int sysId): Dispatches a signal to the node identified by the given system ID. The transmission of the signal occurs through unicast. This explanation refrains from replicating existing content.

## Team Members

- Panchumarthi Venkata Siva Sai (20CS01036)
- N.Ganesh(20CS01038)
- M.Venkata Ramana(20CS01033)

