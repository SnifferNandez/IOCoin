// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IOCOIN_ROSETTA_SERVER_H
#define IOCOIN_ROSETTA_SERVER_H

#include "rosetta.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <functional>

namespace Rosetta {

// Class to handle HTTP server functionality for the Rosetta API
class RosettaServer {
public:
    RosettaServer(uint16_t port);
    ~RosettaServer();

    // Start the server
    bool Start();
    
    // Stop the server
    void Stop();
    
    // Check if server is running
    bool IsRunning() const;

private:
    // Server state
    bool running;
    
    // Port to listen on
    uint16_t port;
    
    // Server thread
    boost::thread server_thread;
    
    // IO Service for asynchronous operations
    boost::asio::io_service io_service;
    
    // Thread-safe shutdown flag
    std::atomic<bool> shutdown_requested;
    
    // Server main loop
    void ServerMain();
    
    // Handle an incoming connection
    void HandleConnection(boost::asio::ip::tcp::socket socket);
    
    // Parse HTTP request and dispatch to appropriate handler
    void HandleRequest(const std::string& request, std::string& response);
    
    // Route the request to the correct API handler based on the endpoint
    Response RouteRequest(const std::string& endpoint, const json_spirit::Object& request_body);
};

} // namespace Rosetta

#endif // IOCOIN_ROSETTA_SERVER_H
