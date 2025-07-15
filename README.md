# Secure Chat Application

A multi-client chat application implemented in C with XOR encryption for secure communication between clients.

## Table of Contents
- [Objective](#objective)
- [Features](#features)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
- [Prerequisites](#prerequisites)
- [Installation & Setup](#installation--setup)
- [Usage](#usage)
- [Technical Details](#technical-details)
- [Security Considerations](#security-considerations)
- [Contributing](#contributing)
- [License](#license)

## Objective

The Secure Chat Application aims to provide a real-time, multi-client chat system with basic encryption capabilities. This project demonstrates:

- **Network Programming**: Implementation of TCP socket communication
- **Concurrent Programming**: Handling multiple clients simultaneously
- **Basic Cryptography**: XOR cipher for message encryption
- **System Programming**: Using poll() for efficient I/O multiplexing

## Features

✅ **Multi-client Support**: Up to 100 concurrent clients  
✅ **Real-time Messaging**: Instant message broadcasting  
✅ **XOR Encryption**: All messages encrypted before transmission  
✅ **User Management**: Username-based client identification  
✅ **Join/Leave Notifications**: Automatic announcements when users connect/disconnect  
✅ **Cross-platform**: Works on Linux and other Unix-like systems  

## Project Structure

```
secure-chat/
├── README.md              # Project documentation
├── client/
│   └── client.c          # Client-side application
└── server/
    └── server.c          # Server-side application
```

## How It Works

1. **Server**: Listens for incoming connections and manages multiple clients using poll()
2. **Client**: Connects to server, sends/receives encrypted messages via threads
3. **Encryption**: All messages are encrypted using XOR cipher before transmission
4. **Broadcasting**: Server forwards messages to all connected clients except sender

## Prerequisites

- **GCC Compiler** (or any C compiler)
- **POSIX-compliant system** (Linux, macOS, etc.)
- **pthread library** (usually included by default)

## Installation & Setup

### 1. Clone the Repository

git clone https://github.com/binayakjoshi/secure-chat.git
cd secure-chat


### 2. Compile the Applications

**Compile Server:**
```bash
cd server
gcc -o server server.c
```

**Compile Client:**
```bash
cd ../client
gcc -o client client.c -lpthread
```

## Usage

### Starting the Server

1. Navigate to the server directory:
```bash
cd server
```

2. Run the server with a port number:
```bash
./server 8080
```

You should see:
```
Server listening on port 8080
```

### Connecting Clients

1. Open a new terminal and navigate to the client directory:
```bash
cd client
```

2. Connect to the server:
```bash
./client 127.0.0.1 8080
```

3. Enter your username when prompted:
```
Enter username: Alice
```

4. Start chatting! Type messages and press Enter:
```
Enter message: Hello everyone!
```

### Multiple Clients

Repeat the client connection steps in separate terminals to simulate multiple users chatting simultaneously.

## Technical Details

### Server Architecture
- **Socket Programming**: TCP sockets for reliable communication
- **I/O Multiplexing**: `poll()` system call for handling multiple clients
- **Client Management**: Array-based client tracking with username storage
- **Message Broadcasting**: Encrypted message forwarding to all active clients

### Client Architecture
- **Threading**: Separate thread for receiving messages
- **Encryption**: XOR cipher applied before sending messages
- **User Interface**: Simple command-line interface for messaging

### Encryption Method
- **Algorithm**: XOR cipher with key `0x5A`
- **Application**: Applied to all message content before transmission
- **Decryption**: Same XOR operation reverses the encryption

### Work Flow
```
1. Client connects to server
2. Client sends encrypted join message: "<username> has joined"
3. Server broadcasts join notification to all other clients
4. Regular chat messages follow format: "<username>: <message>"
5. All messages encrypted with XOR before transmission
6. Server decrypts and re-encrypts for broadcast
7. Client disconnection triggers leave notification
```

## Security Considerations

**Important**: This implementation uses a simple XOR cipher for demonstration purposes. For production use, consider:

- **Stronger Encryption**: Implement AES or other robust encryption algorithms
- **Key Management**: Secure key exchange mechanisms
- **Authentication**: User authentication and authorization
- **Transport Security**: SSL/TLS for additional transport layer security

## How can you help us with editing the code :

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Commit your changes (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Open a Pull Request



