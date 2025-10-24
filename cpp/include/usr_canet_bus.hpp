#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <chrono>
#include <optional>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace usr_canet {

// CAN message structure matching python-can format
struct CanMessage {
    uint32_t arbitration_id;
    bool is_extended_id;
    bool is_remote_frame;
    bool is_error_frame;
    std::vector<uint8_t> data;
    double timestamp;
    
    CanMessage() : arbitration_id(0), is_extended_id(false), 
                   is_remote_frame(false), is_error_frame(false),
                   timestamp(0.0) {}
};

class UsrCanetBus {
public:
    /**
     * @brief Construct a new UsrCanetBus object
     * @param host IP address of USR-CANET200 device
     * @param port TCP port of the CAN bus on the device
     * @param reconnect Whether to auto-reconnect on disconnect
     * @param reconnect_delay Seconds to wait before reconnecting
     */
    UsrCanetBus(const std::string& host = "192.168.0.7",
                uint16_t port = 20001,
                bool reconnect = true,
                int reconnect_delay = 2);
    
    ~UsrCanetBus();
    
    // Delete copy constructor and assignment
    UsrCanetBus(const UsrCanetBus&) = delete;
    UsrCanetBus& operator=(const UsrCanetBus&) = delete;
    
    /**
     * @brief Send a CAN message
     * @param msg The CAN message to send
     * @param timeout Timeout in seconds (0 = no timeout)
     * @return true if sent successfully
     */
    bool send(const CanMessage& msg, double timeout = 0.0);
    
    /**
     * @brief Receive a CAN message
     * @param timeout Timeout in seconds (0 = block indefinitely)
     * @return Optional CAN message (nullopt on timeout or error)
     */
    std::optional<CanMessage> recv(double timeout = 1.0);
    
    /**
     * @brief Check if connected to the device
     */
    bool is_connected() const { return connected_; }
    
    /**
     * @brief Get the socket file descriptor (for advanced use)
     */
    int get_socket_fd() const { return socket_fd_; }
    
    /**
     * @brief Shutdown and close connection
     */
    void shutdown();

private:
    // Connection management
    bool connect();
    void do_reconnect();
    void set_keepalive();
    void set_socket_timeout(double timeout);
    
    // CAN message encoding/decoding
    std::vector<uint8_t> encode_message(const CanMessage& msg);
    std::optional<CanMessage> decode_message(const uint8_t* data, size_t len);
    
    // Configuration
    std::string host_;
    uint16_t port_;
    bool reconnect_;
    int reconnect_delay_;
    
    // Socket state
    int socket_fd_;
    bool connected_;
    
    // Timing
    static double get_timestamp();
};

} // namespace usr_canet
