#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int opt = 1;
    // Create socket
    server_fd = socket( AF_INET,      // Domain: IPv4 internet protocol
                        SOCK_STREAM,  // Type: TCP (reliable stream)
                        0);           // Protocol: 0 = "use default for SOCK_STREAM" (TCP)
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    // Set socket options (SO_REUSEADDR)

    if (setsockopt(server_fd,     // Which socket? 
           SOL_SOCKET,    // What level?  (socket level, not TCP/IP level)
           SO_REUSEADDR,  // What option?  (reuse address)
           &opt,          // Pointer to value (1 = enable, 0 = disable)
           sizeof(opt)) < 0){  // Size of the value {
        perror("Set socket options failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }   
    // TODO: Setup server address structure
    
    // TODO:  Bind socket to port
    
    // TODO: Listen for connections
    
    printf("Server listening on port %d...\n", PORT);
    
    // TODO: Main loop - accept, read, write, close
    
    return 0;
}