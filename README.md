# HTTP Server from Scratch

Building a fully functional HTTP server in C to understand network programming, the HTTP protocol, and concurrent system architectures.

## Project Goals

- Understand socket programming at a fundamental level
- Implement HTTP/1.1 protocol from scratch
- Learn different concurrency models (iterative, forking, threading, event-driven)
- Build something useful while mastering systems programming

## Learning Objectives

By the end of this project, you will understand:
- TCP/IP socket programming (bind, listen, accept, read/write)
- HTTP protocol structure (requests, responses, headers, status codes)
- File I/O and efficient data transfer
- Process and thread management
- Event-driven I/O with epoll/poll
- Resource management and error handling

## Development Roadmap

### Phase 1: Foundation - Echo Server
**Goal**:  Establish basic socket communication

**Core Concepts**:
- Socket creation and configuration
- Binding to a port
- Listening for connections
- Accepting client connections
- Reading and writing data
- Connection lifecycle management

**Deliverable**: A server that echoes back whatever the client sends

---

### Phase 2: HTTP Basics - Hello World Server
**Goal**: Speak HTTP

**Core Concepts**:
- HTTP request structure (method, path, version, headers)
- HTTP response structure (status line, headers, body)
- Request parsing (extracting method and path)
- Response formatting
- Content-Type headers
- Content-Length calculation

**Deliverable**: A server that returns a hardcoded HTML page with proper HTTP headers

---

### Phase 3: Static File Server
**Goal**: Serve real files from disk

**Core Concepts**: 
- File system operations (stat, open, read)
- Path handling and security (prevent directory traversal)
- MIME type detection based on file extensions
- Efficient file transfer (sendfile system call)
- Directory index files (serving index.html for directories)

**Deliverable**: A server that serves HTML, CSS, JavaScript, images, and other static assets

---

### Phase 4: HTTP Status Codes & Error Handling
**Goal**: Handle edge cases gracefully

**Core Concepts**: 
- 200 OK - Successful requests
- 404 Not Found - Missing resources
- 400 Bad Request - Malformed requests
- 500 Internal Server Error - Server failures
- Custom error pages
- Robust error handling and resource cleanup

**Deliverable**: A server with proper error responses and graceful failure handling

---

### Phase 5: Enhanced HTTP Features
**Goal**: Support more HTTP functionality

**Core Concepts**: 
- Request header parsing (Host, User-Agent, Accept, etc.)
- Response headers (Date, Server, Connection, etc.)
- Keep-Alive vs Connection: close
- URL decoding
- Query string parsing
- Multiple HTTP methods (HEAD, POST)

**Deliverable**: A more complete HTTP/1.1 implementation

---

### Phase 6: Concurrency - Forking Model
**Goal**: Handle multiple clients simultaneously

**Core Concepts**: 
- Process forking with fork()
- Parent and child process responsibilities
- Zombie process prevention
- Signal handling (SIGCHLD)
- Process resource management
- Pros and cons of the forking model

**Deliverable**: A multi-process server using fork()

---

### Phase 7: Concurrency - Threading Model
**Goal**: Alternative concurrency approach

**Core Concepts**: 
- POSIX threads (pthread)
- Thread creation and joining
- Thread-per-connection model
- Thread pools for resource management
- Thread safety and shared state
- Mutexes and synchronization (if needed)
- Pros and cons vs forking

**Deliverable**: A multi-threaded server using pthreads

---

### Phase 8: Concurrency - Event-Driven I/O
**Goal**: Maximum scalability with non-blocking I/O

**Core Concepts**:
- Non-blocking sockets
- I/O multiplexing with poll() or select()
- Edge-triggered vs level-triggered events
- Linux epoll() for high performance
- Event loop architecture
- State machines for connection handling
- The C10K problem and solutions

**Deliverable**: A single-threaded event-driven server using epoll

---

### Phase 9: POST Method & Form Handling
**Goal**: Accept data from clients

**Core Concepts**:
- Reading request body
- Content-Length driven reads
- application/x-www-form-urlencoded parsing
- multipart/form-data (file uploads)
- Creating dynamic responses
- Basic routing system

**Deliverable**: A server that can process HTML form submissions

---

### Phase 10: Advanced Features (Optional Extensions)
**Goal**: Polish and production-readiness

**Possible Features**:
- Configuration file support
- Request/access logging
- Virtual hosts
- Basic authentication
- HTTPS/TLS support (with OpenSSL)
- Compression (gzip)
- Caching headers (ETag, Last-Modified)
- Range requests (partial content)
- CGI support
- WebSocket upgrade
- Rate limiting
- Graceful shutdown
- Daemonization
- chroot for security

**Deliverable**: A feature-rich, production-capable HTTP server

---

## Repository Structure

```
http-server/
├── README.md                 # This file
├── Makefile                  # Build automation
├── src/
│   ├── 01_echo/             # Phase 1: Echo server
│   ├── 02_http_basic/       # Phase 2: Basic HTTP
│   ├── 03_static_files/     # Phase 3: File serving
│   ├── 04_error_handling/   # Phase 4: Status codes
│   ├── 05_http_enhanced/    # Phase 5: More HTTP
│   ├── 06_forking/          # Phase 6: Multi-process
│   ├── 07_threading/        # Phase 7: Multi-threaded
│   ├── 08_event_driven/     # Phase 8: epoll
│   ├── 09_post_forms/       # Phase 9: POST handling
│   └── 10_advanced/         # Phase 10: Extensions
├── public/                   # Static files to serve
│   ├── index.html
│   ├── css/
│   ├── js/
│   └── images/
├── tests/                    # Test scripts
└── docs/                     # Additional documentation
```

## Resources

### Essential Reading
- Beej's Guide to Network Programming
- "The Linux Programming Interface" by Michael Kerrisk
- RFC 2616 (HTTP/1.1) - at least sections 4, 5, 6, 9, 10
- "Unix Network Programming" by W. Richard Stevens

### Helpful Tools
- `curl` - Test HTTP requests
- `telnet` / `nc` (netcat) - Raw socket testing
- `strace` - Debug system calls
- `valgrind` - Memory leak detection
- `ab` (Apache Bench) - Performance testing
- `wireshark` - Packet inspection

### Reference Implementations
- zerohttpd - Teaching-focused examples
- nginx source code - Production server architecture
- thttpd - Minimalist approach

##  Testing Strategy

- **Unit**:  Test individual components (parsing, header generation)
- **Integration**: Test full request/response cycles
- **Load**:  Test with many concurrent connections
- **Security**: Test for directory traversal, buffer overflows
- **Compliance**: Test against HTTP spec requirements

##  Success Metrics

- [ ] Can serve a static website with HTML, CSS, JS, images
- [ ] Handles 100+ concurrent connections
- [ ] Properly implements HTTP/1.1 core features
- [ ] No memory leaks (valgrind clean)
- [ ] Graceful error handling
- [ ] Can view in a real web browser
- [ ] Passes basic HTTP compliance tests

##  Getting Started

1. Start with Phase 1 - get the echo server working
2. Test thoroughly before moving to the next phase
3. Keep each phase in a separate directory for comparison
4. Document your learnings and challenges
5. Experiment and break things - that's how you learn!

##  Development Log

Track your progress, challenges encountered, and lessons learned as you work through each phase.

---

**Current Phase**: Phase 1 - Echo Server  
**Status**: In Progress  
**Next Milestone**: Accept and echo back TCP connections