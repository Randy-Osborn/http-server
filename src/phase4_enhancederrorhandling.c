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
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

//get current timestamp for logging
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}
// Helper function to send HTTP error responses
void send_error_response(int client_fd, int status_code, const char *status_message) {
    // Build error file path based on status code
    char error_file_path[256];
    snprintf(error_file_path, sizeof(error_file_path), "./errors/%d.html", status_code);
    
    // Try to read the error HTML file
    struct stat file_stat;
    if (stat(error_file_path, &file_stat) < 0) {
        // Fallback: simple error if custom page doesn't exist
        const char *fallback = "<html><body><h1>Error</h1><p>An error occurred.</p></body></html>";
        char response[512];
        snprintf(response, sizeof(response),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n%s",
            status_code, status_message, strlen(fallback), fallback);
        write(client_fd, response, strlen(response));
        return;
    }
    
    // Open and read error file
    int file_fd = open(error_file_path, O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open error page");
        return;
    }
    
    char *error_html = malloc(file_stat.st_size + 1);
    if (!error_html) {
        close(file_fd);
        return;
    }
    
    ssize_t bytes_read = read(file_fd, error_html, file_stat.st_size);
    close(file_fd);
    error_html[file_stat.st_size] = '\0';  // Null terminate
    
    if (bytes_read < 0) {
        free(error_html);
        return;
    }
    
    // Build and send HTTP response
    char headers[512];
    int header_len = snprintf(headers, sizeof(headers),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_message, file_stat.st_size);
    
    // Send headers
    write(client_fd, headers, header_len);
    // Send HTML body
    write(client_fd, error_html, file_stat.st_size);
    
    free(error_html);
    printf("Sent %d %s response\n", status_code, status_message);
}


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
           sizeof(opt)) < 0){  // Size of the value 
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
    
    // Main loop - accept requests and serve files
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
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        printf("[%s] Connection from %s:%d\n", timestamp, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
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
            int parsed = sscanf(buffer, "%s %s %s", method, path, version);
            
            // Validate HTTP request format (400 Bad Request)
            // Check if parsing was successful (should get 3 items)
            if (parsed < 3) {
                printf("Invalid request format - missing fields\n");
                send_error_response(client_fd, 400, "Bad Request");
                close(client_fd);
                continue;
            }
            
            // Check for valid HTTP method (GET, POST, HEAD)
            if (strcmp(method, "GET") != 0 && 
                strcmp(method, "POST") != 0 && 
                strcmp(method, "HEAD") != 0) {
                printf("Invalid HTTP method: %s\n", method);
                send_error_response(client_fd, 400, "Bad Request");
                close(client_fd);
                continue;
            }
            
            // Check for valid HTTP version (HTTP/1.0 or HTTP/1.1)
            if (strcmp(version, "HTTP/1.0") != 0 && 
                strcmp(version, "HTTP/1.1") != 0) {
                printf("Invalid HTTP version: %s\n", version);
                send_error_response(client_fd, 400, "Bad Request");
                close(client_fd);
                continue;
            }
            
            printf("Method: %s, Path: %s, Version: %s\n", method, path, version);
          
            // Path traversal security check
            // Prevent access to files outside public/ directory
            if (strstr(path, "..") != NULL) {
                printf("Path traversal attempt detected: %s\n", path);
                send_error_response(client_fd, 400, "Bad Request");
                close(client_fd);
                continue;
            }
          
            // build file path            
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "./public%s", path);
            
            // If path ends with '/' or is just '/', append 'index.html'
            if (path[strlen(path) - 1] == '/') {
                strncat(file_path, "index.html", sizeof(file_path) - strlen(file_path) - 1);
            }
            
            printf("looking for file: %s\n", file_path);
            
            //check file existence
            struct stat file_stat;
            if(stat(file_path, &file_stat) < 0){
                //404 Not Found handling
                printf("File not found: %s\n", file_path);
                send_error_response(client_fd, 404, "Not Found");
                close(client_fd);
                continue;
            }
            printf("File found, size: %ld bytes\n", file_stat.st_size);
            //Read file contents
            int file_fd = open(file_path, O_RDONLY); //read only
            //Error check each step
            if(file_fd < 0){
                //File exists but can't open it - server error
                perror("File open failure");
                send_error_response(client_fd, 500, "Internal Server Error");
                close(client_fd);
                continue;
            }
            printf("File opened successfully\n");

            char *file_buffer = malloc(file_stat.st_size);
            if(!file_buffer){
                //Can't allocate memory - server error
                perror("Memory allocation failure");
                send_error_response(client_fd, 500, "Internal Server Error");
                close(file_fd);
                close(client_fd);
                continue;
            }
            ssize_t total_bytes_read = read(file_fd, file_buffer, file_stat.st_size);
            close(file_fd);
            if(total_bytes_read < 0 || total_bytes_read < (ssize_t)file_stat.st_size){
                //Can't read file contents - server error
                perror("File read failure");
                send_error_response(client_fd, 500, "Internal Server Error");
                free(file_buffer);
                close(client_fd);
                continue;
            }
            printf("File read successfully\n");


            //Detect MIME type
            
            const char *content_type = "application/octet-stream";  // Default for unknown types
            // find the file extension
            const char *ext = strrchr(file_path, '.');
            if(ext){
                if(strcmp(ext, ".html") == 0){
                    content_type = "text/html";
                }else if(strcmp(ext, ".css") == 0){
                    content_type = "text/css";
                }else if(strcmp(ext, ".js") == 0){
                    content_type = "application/javascript";
                }else if(strcmp(ext, ".png") == 0){
                    content_type = "image/png";
                }else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0){
                    content_type = "image/jpeg";
                }else if(strcmp(ext, ".gif") == 0){
                    content_type = "image/gif";
                }   else if(strcmp(ext, ".txt") == 0){
                    content_type = "text/plain";
                }
            }
            printf("Detected Content-Type: %s\n", content_type);


            //Build HTTP response
            char header_buffer[BUFFER_SIZE];
            int header_length = snprintf(header_buffer, sizeof(header_buffer),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n", content_type, file_stat.st_size);
            if(header_length < 0 || header_length >= BUFFER_SIZE){
                perror("Header formatting failure");
                free(file_buffer);
                close(client_fd);
                continue;
            }

            //Send response
            //send headers
            ssize_t header_bytes_written = write(client_fd, header_buffer, header_length);
            if(header_bytes_written < 0 || header_bytes_written < header_length){
                perror("Header write failure");
                free(file_buffer);
                close(client_fd);
                continue;
            }
            printf("Headers sent successfully\n");
            //send file content
            ssize_t file_bytes_written = write(client_fd, file_buffer, file_stat.st_size);
            if(file_bytes_written < 0 || file_bytes_written < (ssize_t)file_stat.st_size){
                perror("File content write failure");
                free(file_buffer);
                close(client_fd);
                continue;
            }
            printf("File content sent successfully\n");
            free(file_buffer);
            printf("[%s] 200 OK - Served %s\n", timestamp, file_path);
            close(client_fd);
        }

    }
    return 0;
}