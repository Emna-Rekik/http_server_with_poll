#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define HTML_FILE "index.html"

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Read the HTML file
    int html_fd = open(HTML_FILE, O_RDONLY);
    struct stat html_stat;
    fstat(html_fd, &html_stat);
    size_t html_size = html_stat.st_size;
    char *html_data = malloc(html_size + 1);
    read(html_fd, html_data, html_size);
    html_data[html_size] = '\0';
    close(html_fd);

    // Send the HTTP response headers with Content-Length
    char http_response[BUFFER_SIZE];
    sprintf(http_response, "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %zu\r\n\r\n",
        html_size);

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Initialize an array of pollfd structures for polling events
    struct pollfd fds[MAX_CLIENTS];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        // Poll for events on the file descriptors
        int ret = poll(fds, MAX_CLIENTS, -1);
        if (ret < 0) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        // Iterate through the file descriptors
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (fds[i].revents & POLLIN) {
                // If event is on the server socket, accept
                if (i == 0) {
                    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    if (new_socket < 0) {
                        perror("accept failed");
                        exit(EXIT_FAILURE);
                    }

                    // Find an empty slot in the fds array and add the new client socket
                    for (int j = 1; j < MAX_CLIENTS; j++) {
                        if (fds[j].fd == 0) {
                            fds[j].fd = new_socket;
                            fds[j].events = POLLIN;
                            break;
                        }
                    }
                } else {
                    
                    // Clear the buffer
                    memset(buffer, 0, BUFFER_SIZE);
                    
                    // Read any remaining data from the client
                    int valread = read(fds[i].fd, buffer, BUFFER_SIZE);
                    if (valread > 0) {
                        // Send the HTTP response headers
                        send(fds[i].fd, http_response, strlen(http_response), 0);
                    }
                    
                    // Send the HTML content to the client
                    send(fds[i].fd, html_data, html_size, 0);

                    // Close the client socket and clear its entry in the fds array
                    close(fds[i].fd);
                    fds[i].fd = 0;
                }
            }
        }
    }

    free(html_data); // Free the HTML data
    return 0;
}
