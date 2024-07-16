# C-Chat-App

This repository contains a simple chat application implemented in C. It includes both server and client programs that allow two clients to communicate with each other through the server.

## Files

- **client_chat.c**: The client-side application code.
- **server_chat.c**: The server-side application code.

## Features

- The server accepts connections from two clients.
- Once both clients are connected, they can send and receive messages to and from each other.
- The server manages the connections and relays messages between the clients.
- Clients can exit the chat by typing 'exit' or pressing `Ctrl+C`.

## Prerequisites

- A C compiler (e.g., `gcc`).
- Basic knowledge of socket programming in C.

## Getting Started

### Compiling the Code

To compile the client and server programs, run the following commands:

```sh
gcc -o server server_chat.c
gcc -o client client_chat.c
```

### Running the Server

Start the server by specifying the port number:

```sh
./server <port_number>
```

### Running the Client

Start each client and connect to the server by specifying the server IP and port number:

```sh
./client <server_ip> <port_number>
```

## Code Overview

### client_chat.c

The client program performs the following tasks:

- Connects to the server.
- Reads user input and sends messages to the server.
- Receives messages from the server and displays them.
- Handles exit commands and signals for graceful shutdown.

### server_chat.c

The server program performs the following tasks:

- Binds to a specified port and listens for incoming connections.
- Accepts connections from two clients.
- Relays messages between the connected clients.
- Handles client disconnections and shutdown gracefully.

## Error Handling

Both client and server programs include basic error handling to manage connection issues and unexpected termination.

## License

This project is licensed under the MIT License. 
