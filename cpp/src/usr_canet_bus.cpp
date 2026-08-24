#include "usr_canet_bus.hpp"
#include <stdexcept>
#include <cstring>
#include <thread>
#include <iostream>
#include <iomanip>

namespace usr_canet {

UsrCanetBus::UsrCanetBus(const std::string& host, uint16_t port,
                         bool reconnect, int reconnect_delay)
    : host_(host), port_(port), reconnect_(reconnect),
      reconnect_delay_(reconnect_delay), socket_fd_(-1), connected_(false) {
    
    if (!connect()) {
        throw std::runtime_error("Failed to connect to USR-CANET device at " + 
                                host + ":" + std::to_string(port));
    }
}

UsrCanetBus::~UsrCanetBus() {
    shutdown();
}

bool UsrCanetBus::connect() {
    while (!connected_) {
        // Create socket
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            if (!reconnect_) return false;
            std::this_thread::sleep_for(std::chrono::seconds(reconnect_delay_));
            continue;
        }
        
        // Set up server address
        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        
        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            throw std::runtime_error("Invalid IP address: " + host_);
        }
        
        // Connect to server
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, 
                     sizeof(server_addr)) < 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
            
            if (!reconnect_) {
                return false;
            }
            
            std::cerr << "Could not connect to " << host_ << ":" << port_ 
                     << ". Retrying in " << reconnect_delay_ << "s..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(reconnect_delay_));
            continue;
        }
        
        // Set TCP keepalive
        set_keepalive();
        connected_ = true;
        
        std::cout << "Connected to USR-CANET device at " << host_ 
                 << ":" << port_ << std::endl;
        return true;
    }
    
    return false;
}

void UsrCanetBus::set_keepalive() {
    int keepalive = 1;
    int keepidle = 1;    // Start keepalive after 1s idle
    int keepintvl = 3;   // Send keepalive every 3s
    int keepcnt = 5;     // Close after 5 failed pings
    
    setsockopt(socket_fd_, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    setsockopt(socket_fd_, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(socket_fd_, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(socket_fd_, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
}

void UsrCanetBus::set_socket_timeout(double timeout) {
    struct timeval tv;
    
    if (timeout > 0) {
        tv.tv_sec = static_cast<long>(timeout);
        tv.tv_usec = static_cast<long>((timeout - tv.tv_sec) * 1000000);
    } else {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
    
    setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

void UsrCanetBus::do_reconnect() {
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
    
    if (reconnect_) {
        std::cerr << "Connection lost. Reconnecting..." << std::endl;
        connect();
    }
}

bool UsrCanetBus::send(const CanMessage& msg, double timeout) {
    if (!connected_) return false;
    
    // Set timeout if specified
    if (timeout > 0) {
        set_socket_timeout(timeout);
    }
    
    // Encode message
    auto raw_data = encode_message(msg);
    
    // Send data
    ssize_t bytes_sent = ::send(socket_fd_, raw_data.data(), raw_data.size(), 0);
    
    // Reset timeout
    if (timeout > 0) {
        set_socket_timeout(0);
    }
    
    if (bytes_sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Timeout
            return false;
        }
        // Socket error
        std::cerr << "Socket send error: " << strerror(errno) << std::endl;
        do_reconnect();
        return false;
    }
    
    return bytes_sent == static_cast<ssize_t>(raw_data.size());
}

std::optional<CanMessage> UsrCanetBus::recv(double timeout) {
    if (!connected_) return std::nullopt;
    
    // Set timeout
    set_socket_timeout(timeout);
    
    int timeout_count = 0;
    const int max_timeouts = 10;
    
    while (timeout_count < max_timeouts) {
        uint8_t buffer[13];  // USR-CANET always sends 13 bytes per CAN packet
        
        ssize_t bytes_received = ::recv(socket_fd_, buffer, sizeof(buffer), 0);
        
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Timeout - sleep briefly to prevent CPU spinning
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                timeout_count++;
                continue;
            }
            
            // Socket error
            std::cerr << "Socket recv error: " << strerror(errno) << std::endl;
            do_reconnect();
            return std::nullopt;
        }
        
        if (bytes_received == 0) {
            // Connection closed
            std::cerr << "Connection closed by peer" << std::endl;
            do_reconnect();
            return std::nullopt;
        }
        
        // Decode and return message
        return decode_message(buffer, bytes_received);
    }
    
    // Max timeouts reached
    return std::nullopt;
}

std::vector<uint8_t> UsrCanetBus::encode_message(const CanMessage& msg) {
    std::vector<uint8_t> raw(13, 0);
    
    // Byte 0: Frame info (DLC in bits 0-3, remote frame in bit 6, extended ID in bit 7)
    uint8_t frame_info = static_cast<uint8_t>(msg.data.size() & 0x0F);  // DLC in lower 4 bits
    if (msg.is_extended_id) {
        frame_info |= 0x80;  // Set bit 7 for extended ID
    }
    if (msg.is_remote_frame) {
        frame_info |= 0x40;  // Set bit 6 for remote frame
    }
    raw[0] = frame_info;
    
    // Bytes 1-4: CAN ID (big-endian)
    raw[1] = (msg.arbitration_id >> 24) & 0xFF;
    raw[2] = (msg.arbitration_id >> 16) & 0xFF;
    raw[3] = (msg.arbitration_id >> 8) & 0xFF;
    raw[4] = msg.arbitration_id & 0xFF;
    
    // Bytes 5-12: Data (up to 8 bytes, rest padded with 0)
    for (size_t i = 0; i < msg.data.size() && i < 8; ++i) {
        raw[5 + i] = msg.data[i];
    }
    
    return raw;
}

std::optional<CanMessage> UsrCanetBus::decode_message(const uint8_t* data, size_t len) {
    if (len < 13) {
        return std::nullopt;  // Invalid packet
    }
    
    CanMessage msg;
    msg.timestamp = get_timestamp();
    
    // Byte 0: Frame info (contains frame type, DLC, extended ID flag, remote frame flag)
    uint8_t frame_info = data[0];
    
    // Extract DLC (last 4 bits of byte 0)
    uint8_t data_len = frame_info & 0x0F;
    if (data_len > 8) data_len = 8;  // Clamp to max CAN data length
    
    // Extract extended ID flag (bit 7)
    msg.is_extended_id = (frame_info & 0x80) != 0;
    
    // Extract remote frame flag (bit 6)
    msg.is_remote_frame = (frame_info & 0x40) != 0;
    // Extract remote frame flag (bit 6)
    msg.is_remote_frame = (frame_info & 0x40) != 0;
    
    // Bytes 1-4: CAN ID (big-endian)
    msg.arbitration_id = (static_cast<uint32_t>(data[1]) << 24) |
                         (static_cast<uint32_t>(data[2]) << 16) |
                         (static_cast<uint32_t>(data[3]) << 8) |
                         static_cast<uint32_t>(data[4]);
    
    // Bytes 5-12: Data (up to 8 bytes, determined by DLC)
    msg.data.reserve(data_len);
    for (uint8_t i = 0; i < data_len; ++i) {
        msg.data.push_back(data[5 + i]);
    }
    
    msg.is_error_frame = false;
    
    return msg;
}

double UsrCanetBus::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration);
    return seconds.count();
}

void UsrCanetBus::shutdown() {
    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
}

} // namespace usr_canet
