// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rosetta_server.h"
#include "data_api.h"
#include "construction_api.h"
#include "../util.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace boost::asio;
using ip::tcp;

namespace Rosetta {

RosettaServer::RosettaServer(uint16_t port) 
    : running(false), port(port), shutdown_requested(false) {
}

RosettaServer::~RosettaServer() {
    if (running) {
        Stop();
    }
}

bool RosettaServer::Start() {
    if (running) {
        return true;
    }
    
    LogPrintf("Starting Rosetta API server on port %u\n", port);
    
    // Start server in a separate thread
    server_thread = boost::thread(boost::bind(&RosettaServer::ServerMain, this));
    
    running = true;
    return true;
}

void RosettaServer::Stop() {
    if (!running) {
        return;
    }
    
    LogPrintf("Shutting down Rosetta API server\n");
    
    // Signal the server to stop
    shutdown_requested = true;
    io_service.stop();
    
    // Wait for server thread to exit
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    running = false;
}

bool RosettaServer::IsRunning() const {
    return running;
}

void RosettaServer::ServerMain() {
    try {
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
        
        while (!shutdown_requested) {
            // Wait for a connection
            tcp::socket socket(io_service);
            acceptor.accept(socket);
            
            // Handle the connection in a new thread
            boost::thread(boost::bind(&RosettaServer::HandleConnection, this, std::move(socket)));
        }
    }
    catch (std::exception& e) {
        LogPrintf("Rosetta API server error: %s\n", e.what());
    }
}

void RosettaServer::HandleConnection(tcp::socket socket) {
    try {
        // Read the HTTP request
        boost::asio::streambuf request_buf;
        boost::asio::read_until(socket, request_buf, "\r\n\r\n");
        
        // Extract the request
        std::string request_str(
            (std::istreambuf_iterator<char>(&request_buf)),
            std::istreambuf_iterator<char>()
        );
        
        // Read any remaining data, if there's a body
        boost::asio::streambuf body_buf;
        
        // Extract Content-Length if present
        boost::regex content_length_regex("Content-Length: ([0-9]+)");
        boost::smatch match;
        
        if (boost::regex_search(request_str, match, content_length_regex)) {
            int content_length = std::stoi(match[1]);
            
            // Read the body
            if (content_length > 0) {
                size_t bytes_to_read = content_length - request_buf.size();
                if (bytes_to_read > 0) {
                    boost::asio::read(socket, body_buf, boost::asio::transfer_exactly(bytes_to_read));
                    
                    // Append the body to the request
                    std::string body_str(
                        (std::istreambuf_iterator<char>(&body_buf)),
                        std::istreambuf_iterator<char>()
                    );
                    request_str += body_str;
                }
            }
        }
        
        // Process the request and generate a response
        std::string response;
        HandleRequest(request_str, response);
        
        // Send the response
        boost::asio::write(socket, boost::asio::buffer(response));
    }
    catch (std::exception& e) {
        LogPrintf("Error handling Rosetta API connection: %s\n", e.what());
    }
}

void RosettaServer::HandleRequest(const std::string& request, std::string& response) {
    // Parse the HTTP request
    std::vector<std::string> lines;
    boost::split(lines, request, boost::is_any_of("\r\n"));
    
    if (lines.empty()) {
        response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        return;
    }
    
    // Parse the request line (GET /path HTTP/1.1)
    std::vector<std::string> request_parts;
    boost::split(request_parts, lines[0], boost::is_any_of(" "));
    
    if (request_parts.size() != 3) {
        response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        return;
    }
    
    std::string method = request_parts[0];
    std::string path = request_parts[1];
    
    // Only accept POST requests for Rosetta API
    if (method != "POST") {
        response = "HTTP/1.1 405 Method Not Allowed\r\nAllow: POST\r\n\r\n";
        return;
    }
    
    // Find where the body starts
    size_t body_start = request.find("\r\n\r\n");
    if (body_start == std::string::npos) {
        response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        return;
    }
    
    body_start += 4;  // Skip over the \r\n\r\n
    
    // Extract the body
    std::string body = request.substr(body_start);
    
    // Parse the JSON body
    json_spirit::Value parsed_body;
    if (!json_spirit::read(body, parsed_body) || parsed_body.type() != json_spirit::obj_type) {
        response = "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\n\r\n{\"code\":400,\"message\":\"Invalid JSON request body\"}";
        return;
    }
    
    // Route the request to the appropriate handler
    Response api_response = RouteRequest(path, parsed_body.get_obj());
    
    // Convert the API response to HTTP response
    std::string response_body;
    
    if (api_response.code != Success) {
        // Error response
        json_spirit::Object error;
        error.push_back(json_spirit::Pair("code", api_response.code));
        error.push_back(json_spirit::Pair("message", api_response.message));
        response_body = json_spirit::write(error);
    } else {
        // Success response
        response_body = json_spirit::write(api_response.result);
    }
    
    // Build the HTTP response
    std::stringstream ss;
    ss << "HTTP/1.1 " << (api_response.code == Success ? "200 OK" : std::to_string(api_response.code) + " Error") << "\r\n";
    ss << "Content-Type: application/json\r\n";
    ss << "Content-Length: " << response_body.length() << "\r\n";
    ss << "\r\n";
    ss << response_body;
    
    response = ss.str();
}

Response RosettaServer::RouteRequest(const std::string& endpoint, const json_spirit::Object& request_body) {
    // Data API routes
    if (endpoint == "/network/list") {
        return DataAPI::NetworkList(request_body);
    } else if (endpoint == "/network/status") {
        return DataAPI::NetworkStatus(request_body);
    } else if (endpoint == "/network/options") {
        return DataAPI::NetworkOptions(request_body);
    } else if (endpoint == "/account/balance") {
        return DataAPI::AccountBalance(request_body);
    } else if (endpoint == "/block") {
        return DataAPI::Block(request_body);
    } else if (endpoint == "/block/transaction") {
        return DataAPI::BlockTransaction(request_body);
    } else if (endpoint == "/mempool") {
        return DataAPI::Mempool(request_body);
    } else if (endpoint == "/mempool/transaction") {
        return DataAPI::MempoolTransaction(request_body);
    }
    
    // Construction API routes
    else if (endpoint == "/construction/derive") {
        return ConstructionAPI::Derive(request_body);
    } else if (endpoint == "/construction/preprocess") {
        return ConstructionAPI::Preprocess(request_body);
    } else if (endpoint == "/construction/metadata") {
        return ConstructionAPI::Metadata(request_body);
    } else if (endpoint == "/construction/payloads") {
        return ConstructionAPI::Payloads(request_body);
    } else if (endpoint == "/construction/parse") {
        return ConstructionAPI::Parse(request_body);
    } else if (endpoint == "/construction/combine") {
        return ConstructionAPI::Combine(request_body);
    } else if (endpoint == "/construction/hash") {
        return ConstructionAPI::Hash(request_body);
    } else if (endpoint == "/construction/submit") {
        return ConstructionAPI::Submit(request_body);
    }
    
    // Unknown endpoint
    Response response;
    response.code = NotFound;
    response.message = "Endpoint not found: " + endpoint;
    return response;
}

} // namespace Rosetta
