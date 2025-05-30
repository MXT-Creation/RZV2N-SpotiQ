/**
 * @file video_server.cpp
 * @copyright Copyright (c) 2022 IMD Technologies. All rights reserved.
 * @author Paul Thomson <pault@imd-tec.com>
 */

#include "video_server.hpp"

#include <iostream>

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

// If we say we are listening for only one connection we get an error here if
// the host has tried more than once to connect. Allow many connect attempts.
constexpr static int kMaxConnectAttempts = 128;


/**
 *
 */
VideoServer::VideoServer(const uint16_t port_number):
    _port_number(port_number)
{
}

/**
 *
 */
void VideoServer::StartServer()
{
    if (_server_fd >= 0)
    {
        close(_server_fd);
    }

    _server_fd = socket(PF_INET, SOCK_STREAM, 0);

    /// @todo Error handling

    int enable = 1;
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    /// @todo Error handling

    memset(&_server_address, 0, sizeof(struct sockaddr_in));

    _server_address.sin_family = AF_INET;
    _server_address.sin_port = htons(_port_number);
    _server_address.sin_addr.s_addr = (INADDR_ANY);

    (void) bind(_server_fd, (struct sockaddr *)&_server_address, sizeof(_server_address));

    /// @todo Error handling
}

/**
 *
 */
void VideoServer::Listen()
{
    // Here we wait for a connection.
    // The host may make several attempts to connect.
    // Lets keep listening until accept().
    do 
    {
        _client_fd = -1;

        std::cout << "Listening for client" << std::endl;

        int err = listen(_server_fd, kMaxConnectAttempts);

        if (err)
        {
            std::cout << "Failed to listen" << std::endl;
        }
        else
        {
            socklen_t address_length = sizeof(_client_address);
            _client_fd = accept(_server_fd, (struct sockaddr *)&_client_address, &address_length);
            if (_client_fd == -1)
            {
                std::cout << "Failed to accept" << std::endl;
            }
        }
    } while (_client_fd == -1);

    /// @todo Error handling

    char ip_address[80];
    inet_ntop(AF_INET, &_client_address.sin_addr, ip_address, sizeof(ip_address));

    std::cout << "Client connected (" << ip_address << ")" << std::endl;

    // Send image resolution

    /// @todo Check return value from write() functions

    uint16_t img_width{static_cast<uint16_t>(_frame_dimensions.width)};
    uint16_t img_height{static_cast<uint16_t>(_frame_dimensions.height)};

    auto len = ::write(_client_fd, reinterpret_cast<char const *>(&img_width),  sizeof(img_width));
    len =      ::write(_client_fd, reinterpret_cast<char const *>(&img_height), sizeof(img_height));
    (void)len;
}

/**
 *
 */
bool VideoServer::ReadyToSend(const ImageData image)
{
    int used, err;
    const uint32_t frame_size = image.len;
    err = ioctl(_client_fd, SIOCOUTQ, &used);   //Check the amount of unsent data in the socket send queue

    if ((err == 0) && (((uint32_t)used) < frame_size))
        return true;
    else
        return false;
}

/**
 *
 */
void VideoServer::SendFrame(const ImageData image)
{
    const uint32_t frame_size = image.len;

    /// @todo We assume both sides have the same endianness...
    /// @todo Check return from write() functions

    auto len = ::write(_client_fd, reinterpret_cast<char const *>(&frame_size), sizeof(frame_size));

    if (len != sizeof(frame_size))
    {
        std::cerr << "Dropped bytes when sending frame size" << std::endl << std::endl;
    }

    len = ::write(_client_fd, image.buffer, frame_size);

    if (len != frame_size)
    {
        std::cerr << "Dropped bytes when sending frame" << std::endl << std::endl;
    }

    _frame_count++;
}
