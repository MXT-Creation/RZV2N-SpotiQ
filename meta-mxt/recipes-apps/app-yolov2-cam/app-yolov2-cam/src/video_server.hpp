/**
 * @file video_server.hpp
 * @copyright Copyright (c) 2022 IMD Technologies. All rights reserved.
 * @author Paul Thomson <pault@imd-tec.com>
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstdint>
#include <memory>

#include "image.hpp"


static constexpr uint16_t DefaultPortNumber { 5000 };

struct ImageData 
{
    uint8_t *buffer;
    size_t len;
};

/**
 * @brief Sends uncompressed video frames to a remote client via TCP
 */
class VideoServer
{
public:

    /**
     * @brief Constructor
     * @param port_number TCP port number
     */
    VideoServer(const uint16_t port_number = DefaultPortNumber);

    /**
     * @brief Set the resolution of the video frames
     * @param dimensions Frame resolution
     */
    void SetDimensions(const ImageDimensions dimensions);

    /**
     * @brief Start the TCP server
     */
    void StartServer();

    /**
     * @brief Listen for client connections
     */
    void Listen();

    /**
     * @brief Send an RGB frame
     * @param frame Uncompressed video frame
     */
    void SendFrame(const ImageData image);

    /**
     * @brief Check the amount of unsent data in the socket send queue
     * @return Ready to write to the socket or not
     */
    bool ReadyToSend(const ImageData image);

    /**
     * @brief Get a running count of the number of frames that have been sent
     * @return Frame count
     */
    uint64_t GetFrameCount() const;

private:

    /// @brief TCP port number
    uint16_t _port_number;

    /// @brief Frame resolution
    ImageDimensions _frame_dimensions { 1280, 720, IMAGE_CHANNEL_BGR};

    /// @brief Server socket file descriptor
    int _server_fd { -1 };

    /// @brief Client socket file descriptor
    int _client_fd { -1 };

    /// @brief Server address
    struct sockaddr_in _server_address;

    /// @brief Client address
    struct sockaddr_in _client_address;

    /// @brief Running frame count
    uint64_t _frame_count { 0 };
};

inline
void VideoServer::SetDimensions(ImageDimensions dimensions)
{
    _frame_dimensions.width = dimensions.width;
    _frame_dimensions.height = dimensions.height;
    _frame_dimensions.channels = dimensions.channels;
}

inline
uint64_t VideoServer::GetFrameCount() const
{
    return _frame_count;
}

