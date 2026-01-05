#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>


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
    // Setup server address structure
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET; // IPv4
   server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address
   server_addr.sin_port = htons(PORT); // Port number in network byte order

       // Bind socket to port
    if(bind(server_fd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        perror("Bind Failure");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if(listen(server_fd, 5) < 0){
        perror("listen Failure");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
 
    printf("Server listening on port %d...\n", PORT);
    
    // Main loop - accept, read, write, close
    while(1){
        printf("Waiting for a new connection...\n");
        fflush(stdout);
        //accept new connection
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr,&client_len);
        if(client_fd < 0){
            perror("Accept Failure");
           continue;
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if(bytes_read < 0){
            perror("Read Failure");
            close(client_fd);
            continue;
        }else if(bytes_read == 0){
            printf("Client disconnected before sending data\n");
            close(client_fd);
            continue;
        }else{
            printf("Received HTTP request:\n%s\n", buffer);
            // parse http request
            char method[16], path[256], version[16];
            sscanf(buffer, "%s %s %s", method, path, version);
            printf("Method: %s, Path: %s, Version: %s\n", method, path, version);
          
            // build file path            
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "./public%s", path);
            printf("looking for file: %s\n", file_path);
            
            //check file existence
            struct stat file_stat;
            if(stat(file_path, &file_stat) < 0){
                //file not found, send 404 response
                const char *not_found_response = 
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 0\r\n"
                "Connection: close\r\n"
                "\r\n";
                close(client_fd);
                continue;
            }
            printf("File found, size: %ld bytes\n", file_stat.st_size);

            // TODO: Step 3 - Read file contents
            // - Open file with open(file_path, O_RDONLY)
            // - Allocate buffer with malloc(file_stat.st_size)
            // - Read file contents with read()
            // - Close file with close()
            
            // TODO: Step 4 - Detect MIME type
            // - Check file extension (.html, .css, .js, .png, .jpg)
            // - Set appropriate Content-Type string
            //   .html -> "text/html"
            //   .css  -> "text/css"
            //   .js   -> "application/javascript"
            
            // TODO: Step 5 - Build HTTP response
            // - Create response with status line, headers, and file contents
            // - Use snprintf to format headers
            // - Use detected Content-Type from step 4
            // - Use file_stat.st_size for Content-Length
            
            // TODO: Step 6 - Send response
            // - write() the complete response to client_fd
            // - Error check for partial writes
            // - free() the file_buffer
            

            close(client_fd);
        }

     

    }
    return 0;
}