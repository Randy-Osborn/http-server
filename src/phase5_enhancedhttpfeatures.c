#define _POSIX_C_SOURCE 200809L
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
#define BUFFER_SIZE 4096
#define MAX_HEADERS 32
#define HEADER_LINE_SIZE 256

// Structure to hold HTTP request headers
typedef struct {
    char name[HEADER_LINE_SIZE];
    char value[HEADER_LINE_SIZE];
} HttpHeader;

typedef struct {
    HttpHeader headers[MAX_HEADERS];
    int header_count;
} HttpRequest;

typedef struct {
    char key[HEADER_LINE_SIZE];
    char value[HEADER_LINE_SIZE];
} QueryParam;
typedef struct {
    QueryParam params[MAX_HEADERS];
    int param_count;
} QueryString;

//get current timestamp for logging
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Get HTTP-formatted date (RFC 1123)
void get_http_date(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);  // Use GMT/UTC time
    strftime(buffer, size, "%a, %d %b %Y %H:%M:%S GMT", t);
}

// URL decode helper function - decodes percent-encoded characters in-place
// Converts %XX hex codes to ASCII characters and + to space
void url_decode(char *path) {
    int i = 0;  // Read position
    int j = 0;  // Write position
    
    while (path[i] != '\0') {
        if (path[i] == '%' && path[i+1] && path[i+2]) {
            // Found %XX - convert hex to character
            char hex[3] = {path[i+1], path[i+2], '\0'};
            path[j] = (char)strtol(hex, NULL, 16);
            i += 3;  // Skip past %XX
            j++;
        } else if (path[i] == '+') {
            // Convert + to space
            path[j] = ' ';
            i++;
            j++;
        } else {
            // Regular character - copy as-is
            path[j] = path[i];
            i++;
            j++;
        }
    }
    path[j] = '\0';  // Null terminate at final length
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
        perror("Memory allocation failed for error page");
        return;
    }
    
    ssize_t bytes_read = read(file_fd, error_html, file_stat.st_size);
    close(file_fd);
    
    if (bytes_read < 0 || bytes_read != file_stat.st_size) {
        free(error_html);
        perror("Failed to read error page");
        return;
    }
    
    error_html[file_stat.st_size] = '\0';  // Null terminate
    
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
/*
// Parse query string from path and populate QueryString struct
// Example: "/search?q=hello&page=2" becomes path="/search" + query params
// Modifies path in-place to remove query string portion
// Returns number of parameters parsed
"/search?q=hello&page=2"
         ↓ (terminate at '?')
path = "/search"
query_start → "q=hello&page=2"
         ↓ (split BY '&')
token 1: "q=hello"
token 2: "page=2"
         ↓ (find '=' in each token)
token 1: "q" = "hello"
         ↓ (copy both)
query->params[0].key = "q"
query->params[0].value = "hello"
*/
int parse_query_string(char *path, QueryString *query) {
    query->param_count = 0;
    
    // Search through path to find '?' character
    for (size_t i = 0; path[i] != '\0'; i++) {
        if (path[i] == '?') {
            // Found query string separator
            // Terminate path at '?' so path only contains the file path
            path[i] = '\0';  // "/search?q=hello" becomes "/search"
            
            // Start parsing query string after the '?'
            char *query_start = path + i + 1;  // Points to "q=hello&page=2"
            
            // Use strtok_r to split query string by '&' delimiter
            char *token;
            char *saveptr = NULL;
            
            // Loop through each parameter (e.g., "q=hello", "page=2")
            token = strtok_r(query_start, "&", &saveptr);
            while (token != NULL && query->param_count < MAX_HEADERS) {
                // Find '=' to split key from value
                char *equal_sign = strchr(token, '=');
                if (equal_sign) {
                    // Calculate lengths using pointer arithmetic
                    size_t key_len = equal_sign - token;       // Length of "q"
                    size_t value_len = strlen(equal_sign + 1); // Length of "hello"
                    
                    // Safety check: ensure fits in our buffers
                    if (key_len < HEADER_LINE_SIZE && value_len < HEADER_LINE_SIZE) {
                        // Copy key (before '=')
                        strncpy(query->params[query->param_count].key, token, key_len);
                        query->params[query->param_count].key[key_len] = '\0';
                        
                        // Copy value (after '=')
                        strncpy(query->params[query->param_count].value, equal_sign + 1, value_len);
                        query->params[query->param_count].value[value_len] = '\0';
                        
                        query->param_count++;
                    }
                }
                
                // Get next token
                token = strtok_r(NULL, "&", &saveptr);
            }
            return query->param_count;
        }
    }
    
    // No '?' found - no query string present
    return 0;
}

// Parse HTTP headers from request buffer
// Returns the number of headers parsed
int parse_http_headers(const char *request_buffer, HttpRequest *request) {
    request->header_count = 0;
    
    // Find the end of the request line (first \r\n)
    const char *header_start = strstr(request_buffer, "\r\n");
    if (!header_start) {
        return 0;  // No headers found
    }
    header_start += 2;  // Skip past the \r\n
    
    // Parse each header line until we hit \r\n\r\n (end of headers)
    while (*header_start != '\0' && request->header_count < MAX_HEADERS) {
        // Check for end of headers (empty line)
        if (strncmp(header_start, "\r\n", 2) == 0) {
            break;  // End of headers section
        }
        
        // Find the colon separator using strchr()
        const char *colon_pos = strchr(header_start, ':');
        if (!colon_pos) {
            break;  // Malformed header line
        }
        
        // Find the end of this line using strstr()
        const char *line_end = strstr(header_start, "\r\n");
        if (!line_end) {
            break;  // Malformed header line   
        }
        
        // Calculate header name length
        int name_len = colon_pos - header_start;
        //Copy header name into request->headers[request->header_count].name
        if(name_len < HEADER_LINE_SIZE){
            strncpy(request->headers[request->header_count].name,header_start,name_len);
            request->headers[request->header_count].name[name_len] = '\0';
        }else{
            //name too long, skip this header
            break;
        }        
        // Find value start position (skip past colon and spaces)
        const char *start_pos = colon_pos + 1;
        while(*start_pos == ' ' || *start_pos == '\t'){
            start_pos++;
        }
        const char *value_start = start_pos;
        // Calculate header value length
        int value_len = line_end - value_start;
        // Copy header value into request->headers[request->header_count].value
      
        if(value_len < HEADER_LINE_SIZE){
            strncpy(request->headers[request->header_count].value,value_start,value_len);
            request->headers[request->header_count].value[value_len] = '\0';
        }else{
            //value too long, skip this header
            break;
        }
        // Increment header counter

        request->header_count++;
        // Move to next line
        header_start = line_end + 2;  // Move to the start of the next header line
    }
    
    return request->header_count;
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
    printf("Phase 5: Enhanced HTTP Features\n");
    
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
            
            // Parse HTTP request line
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
            
            // Parse request headers
            HttpRequest request;
            int header_count = parse_http_headers(buffer, &request);
            printf("Parsed %d headers\n", header_count);
            // Print all parsed headers (for testing/debugging)
            for(int i = 0; i < header_count; i++){
                printf("  %s: %s\n", request.headers[i].name, request.headers[i].value);
            }
            
            // URL decode the path (convert %20 to space, etc.)
            url_decode(path);
            printf("Decoded path: %s\n", path);
            
            // Query string parsing
            QueryString query;
            int param_count = parse_query_string(path, &query);
            printf("Parsed %d query parameters\n", param_count);
            for(int i = 0; i < param_count; i++){
                printf("  %s = %s\n", query.params[i].key, query.params[i].value);
            }
          
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


            // Detect MIME type based on file extension
            const char *content_type = "application/octet-stream";  // Default for unknown types
            const char *ext = strrchr(file_path, '.');
            if(ext){
                if(strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0){
                    content_type = "text/html";
                }else if(strcmp(ext, ".css") == 0){
                    content_type = "text/css";
                }else if(strcmp(ext, ".js") == 0){
                    content_type = "application/javascript";
                }else if(strcmp(ext, ".json") == 0){
                    content_type = "application/json";
                }else if(strcmp(ext, ".xml") == 0){
                    content_type = "application/xml";
                }else if(strcmp(ext, ".png") == 0){
                    content_type = "image/png";
                }else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0){
                    content_type = "image/jpeg";
                }else if(strcmp(ext, ".gif") == 0){
                    content_type = "image/gif";
                }else if(strcmp(ext, ".svg") == 0){
                    content_type = "image/svg+xml";
                }else if(strcmp(ext, ".ico") == 0){
                    content_type = "image/x-icon";
                }else if(strcmp(ext, ".txt") == 0){
                    content_type = "text/plain";
                }else if(strcmp(ext, ".pdf") == 0){
                    content_type = "application/pdf";
                }else if(strcmp(ext, ".zip") == 0){
                    content_type = "application/zip";
                }
            }
            printf("Detected Content-Type: %s\n", content_type);


            //Build HTTP response
            // Generate HTTP date for Date header
            char http_date[128];
            get_http_date(http_date, sizeof(http_date));
            
            char header_buffer[BUFFER_SIZE];
            int header_length = snprintf(header_buffer, sizeof(header_buffer),
                "HTTP/1.1 200 OK\r\n"
                "Date: %s\r\n"
                "Server: MyHTTPServer/1.0\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n", http_date, content_type, file_stat.st_size);
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
            
            // HEAD METHOD HANDLING HERE
            if(strcmp(method, "HEAD") == 0) {
                free(file_buffer);
                printf("[%s] 200 OK - HEAD request for %s\n", timestamp, file_path);
                close(client_fd);
                continue;
            }
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