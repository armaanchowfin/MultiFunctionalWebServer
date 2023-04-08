# Multifunctional Web Server with a custom application-layer protocol.

This is a first attempt at creating a multithreaded client-server application built on TCP, using C. The SocketsAPI is used for networking and PThreads to implement multithreading. A custom layer 5 protocol has also been designed to perform the various erver functionalities.

<for Mythili Ma'am> : A major update will be pushed this weekend to fix the chat and download functions. [7 Apr 2023]

## Prerequisites
1. Ensure you are using Linux (I use Ubuntu 20.04 running on a Virtual Machine)
2. Ensure ```make``` is installed.

## Usage
1. Compile the project via ```make```
2. Run the server program via ```./server <Server Port Number>```.
3. Run the client program in independent shell processes via ```./client <ClientIP> <Server Port Number>```.

## Functionality:
The server has various functionalities. Multiple clients can connect to and interact with the server concurrently. 
Some functions:
1. ```LIST``` : Returns a list of connected clients to the requesting client. (done)
2. ```EXIT``` : Safely disconnects the requesting client from the server. (done)
3. ```CHAT``` : Initiates a chat mode between requesting client and chat request client(s). A preliminary handshake is performed, inspired by the telnet protocol to confirm client availability. 
Server maintains a log of all messages sent and received. 
4. ```UP```   : Uploads a file (any size/ type) to the server. (TODO)
5. ```DOWN``` : Downloads a file (any size/type) from the server. (TODO)

## Architecture Decisions
Server and all clients run _on the same machine_ as independent shell processes. As a result, there are no network-layer effects. Data is routed to and from local kernel buffers, instead of reaching the NIC.

The project is still in development. It has evolved from a simple single-client TCP echo server to a multiclient server with more complex functionalities. 

The most significant change has been the implementation of a ```packet``` struct for message passing instead of raw strings. This setup allows for more uniform control, which is required to implement certain FSMs in the applications.


### Application Layer Protocol:
- An advancement to the previous naive method of sending raw strings through sockets. Various changes imminent.
- Replaces the need for keyword processing and uses flags defined in a  ```packet``` struct. This struct will likely be modified in time.
- Methods implemented for chat handshake, file download/upload authentication. Can be made more featureful by adding encryption.

### Threads setup:

On intialisation, server and client create their own working directories. 
Server process listens for a connect request at the specified port, and spawns a new ```clienthandler``` thread to handle communication with the connected client.
Each client process, on initialisation spawns 2 threads: ```serverresponsehandler``` and ```clientinputhandler```. 
- ```serverresponsehandler``` implements an FSM based on certain ```packet``` flags. Used for chat authentication at present.
- ```clientinputhandler``` gets input from user terminal and sends it to the server for further processing.

### Server-Side User's List:
 - Each connected client is stored as a node in a Doubly-linked list of DLLNode structs.
 - Each node contains thread-local client details.
 - DLL Nodes are accessed instead of multiple File I/O operations to implement ```LIST```.


### Function Details:


## Drawbacks
- No performance testing/optimizations done as yet. 
- Program is not entirely thread-safe yet. Scalability is a concern and will likely lead to the discovery of concurrency bugs.
- Error checking not yet implemented for most functions.




## To be done:
- Formally defining and implementing the Layer-5 protocol : In progress 
- Performance tests: No work done yet.

