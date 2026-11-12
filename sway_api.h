#ifndef SWAY_IPC_H
#define SWAY_IPC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define IPC_MAGIC "i3-ipc"
#define IPC_MAGIC_LEN 6

/* Message types sent to Sway */
#define SWAY_TYPE_COMMAND         0
#define SWAY_TYPE_GET_WORKSPACES  1
#define SWAY_TYPE_SUBSCRIBE       2

/* Event types sent from Sway (Highest bit is set to 1) */
#define SWAY_EVENT_WORKSPACE      (0x80000000 | 0)
#define SWAY_EVENT_OUTPUT         (0x80000000 | 1)
#define SWAY_EVENT_MODE           (0x80000000 | 2)
#define SWAY_EVENT_WINDOW         (0x80000000 | 3)

/* Enforce byte alignment to match the raw socket layout exactly */
#pragma pack(push, 1)
typedef struct {
    char magic[IPC_MAGIC_LEN];
    uint32_t length;
    uint32_t type;
} sway_hdr_t;
#pragma pack(pop)

/**
 * Connects to the active Sway UNIX domain socket.
 * Returns a valid file descriptor on success, or -1 on failure.
 */
static inline int sway_connect(void) {
    char *socket_path = getenv("SWAYSOCK");
    if (!socket_path) {
        socket_path = getenv("I3SOCK");
    }
    if (!socket_path) {
        return -1;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

/**
 * Packs and transmits a message framing header + payload over the socket.
 * Returns 0 on success, or -1 on network failure.
 */
static inline int sway_send(int fd, uint32_t type, const char *payload) {
    uint32_t payload_len = payload ? (uint32_t)strlen(payload) : 0;
    
    sway_hdr_t header;
    memcpy(header.magic, IPC_MAGIC, IPC_MAGIC_LEN);
    header.length = payload_len;
    header.type = type;

    // Send the 14-byte binary header framing
    if (write(fd, &header, sizeof(sway_hdr_t)) != (ssize_t)sizeof(sway_hdr_t)) {
        return -1;
    }

    // Send payload if it exists
    if (payload_len > 0) {
        uint32_t total_sent = 0;
        while (total_sent < payload_len) {
            ssize_t sent = write(fd, payload + total_sent, payload_len - total_sent);
            if (sent <= 0) {
                if (errno == EINTR) continue; // Interrupted by system signal, retry
                return -1;
            }
            total_sent += sent;
        }
    }

    return 0;
}

/**
 * Blocks and reads an incoming message from the socket.
 * Dynamically allocates memory for *out_payload (caller must free it).
 * Returns the message type on success, or -1 on socket drop/failure.
 */
static inline int sway_recv(int fd, uint32_t *out_type, char **out_payload) {
    if (!out_type || !out_payload) return -1;
    
    sway_hdr_t header;
    uint32_t header_bytes_read = 0;

    // Ensure we read the complete 14-byte header across partial TCP/UNIX chunks
    while (header_bytes_read < sizeof(sway_hdr_t)) {
        ssize_t r = read(fd, ((char *)&header) + header_bytes_read, sizeof(sway_hdr_t) - header_bytes_read);
        if (r <= 0) {
            if (r < 0 && errno == EINTR) continue;
            return -1; // Socket closed or errored
        }
        header_bytes_read += r;
    }

    // Validate the magic identifier safely
    if (memcmp(header.magic, IPC_MAGIC, IPC_MAGIC_LEN) != 0) {
        return -1;
    }

    // Allocate storage for payload + 1 extra byte for null-termination
    char *buffer = malloc(header.length + 1);
    if (!buffer) return -1;

    uint32_t payload_bytes_read = 0;
    while (payload_bytes_read < header.length) {
        ssize_t r = read(fd, buffer + payload_bytes_read, header.length - payload_bytes_read);
        if (r <= 0) {
            if (r < 0 && errno == EINTR) continue;
            free(buffer);
            return -1;
        }
        payload_bytes_read += r;
    }
    buffer[header.length] = '\0'; // Safely convert to regular C string

    *out_type = header.type;
    *out_payload = buffer;
    return 0;
}

#endif // SWAY_IPC_H
