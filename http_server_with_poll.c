#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_parser/http_parser.h"
#include "index_html_gz.h"
#include "config.h"

static char url_buffer[MAX_URL_LENGTH];
static char buffer[BUFFER_SIZE] = {0};
static char http_response[BUFFER_SIZE];
static struct pollfd fds[MAX_CLIENTS+1];

static int on_url(http_parser* parser, const char* at, size_t length) {
    // Copy the URL string to the buffer
    strncpy(url_buffer, at, length);
    url_buffer[length] = '\0';
    printf("Requested URL: %s\n", url_buffer);

    return 0;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create a socket
    if ((server_fd = socket(SOCKET_FAMILY, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address and port
    address.sin_family = SOCKET_FAMILY;
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
                    // Create an instance of the http_parser
                    http_parser_settings parserSettings;
                    http_parser parser;
                    http_parser_init(&parser, HTTP_REQUEST);
                    http_parser_settings_init(&parserSettings);
                    parserSettings.on_url = on_url;

                    // Read the data from the client
                    int valread = read(fds[i].fd, buffer, BUFFER_SIZE);
                    if (valread < 0) {
                         perror("read failed");
                         exit(EXIT_FAILURE);
                    }

                    // Parse the HTTP request using the http_parser
                    size_t bytesRead = strlen(buffer);
                    http_parser_execute(&parser, &parserSettings, buffer, bytesRead);

                    if (strcmp(url_buffer, "/") == 0) {
                        // Send the HTTP response headers with Content-Length
                        sprintf(http_response, "HTTP/1.1 200 OK\r\n"
                                                "Content-Type: text/html\r\n"
                                                "Content-Encoding: gzip\r\n"
                                                "Content-Length: %u\r\n\r\n",
                                index_html_gz_len);
                        if (send(fds[i].fd, http_response, strlen(http_response), 0) < 0) {
                            perror("send failed");
                            exit(EXIT_FAILURE);
                        }

                        // Send the compressed HTML content in chunks
                        size_t bytes_sent = 0;
                        while (bytes_sent < index_html_gz_len) {
                            size_t chunk_size = BUFFER_SIZE;
                            if (index_html_gz_len - bytes_sent < chunk_size) {
                                chunk_size = index_html_gz_len - bytes_sent;
                            }
                            if (send(fds[i].fd, index_html_gz + bytes_sent, chunk_size, 0) < 0) {
                                perror("send failed");
                                exit(EXIT_FAILURE);
                            }
                            bytes_sent += chunk_size;
                        }
                    } else {
                        // Resource not found, send a 404 response
                        const char* not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                                         "Content-Length: 9\r\n\r\n"
                                                         "Not Found";
                        if (send(fds[i].fd, not_found_response, strlen(not_found_response), 0) < 0) {
                            perror("send failed");
                            exit(EXIT_FAILURE);
                        }
                    }

                    // Close the client socket and clear its entry in the fds array
                    close(fds[i].fd);
                    fds[i].fd = 0;
                }
            }
        }
    }

    return 0;
}
