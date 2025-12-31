#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in {
    sa_family_t    sin_family;  // AF_INET
    in_port_t      sin_port;    // htons(PORT)
    struct in_addr sin_addr;    // INADDR_ANY
};


#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    
    // TODO: Create socket
    
    // TODO: Set socket options (SO_REUSEADDR)
    
    // TODO: Setup server address structure
    
    // TODO:  Bind socket to port
    
    // TODO: Listen for connections
    
    printf("Server listening on port %d...\n", PORT);
    
    // TODO: Main loop - accept, read, write, close
    
    return 0;
}