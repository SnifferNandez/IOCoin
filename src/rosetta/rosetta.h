// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IOCOIN_ROSETTA_H
#define IOCOIN_ROSETTA_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "../json/json_spirit_reader_template.h"
#include "../json/json_spirit_writer_template.h"
#include "../json/json_spirit_utils.h"

// Rosetta API version
#define ROSETTA_API_VERSION "1.4.13"

namespace Rosetta {

// Forward declarations
class Server;

// Rosetta error codes - mapping to standard HTTP status codes
enum ErrorCode {
    // Standard errors
    Success = 0,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    ServerError = 500,
    NotImplemented = 501,
    
    // Rosetta-specific errors
    InvalidNetworkIdentifier = 1000,
    InvalidAccountIdentifier = 1001,
    InvalidBlockIdentifier = 1002,
    InvalidTransactionIdentifier = 1003
};

// Rosetta standard response structure
struct Response {
    ErrorCode code;
    std::string message;
    json_spirit::Object result;
    
    Response() : code(Success), message("") {}
};

// Rosetta standard types
struct NetworkIdentifier {
    std::string blockchain;
    std::string network;
    std::string sub_network;
    
    json_spirit::Object ToJson() const;
    static NetworkIdentifier FromJson(const json_spirit::Object& obj);
};

struct BlockIdentifier {
    uint64_t index;
    std::string hash;
    
    json_spirit::Object ToJson() const;
    static BlockIdentifier FromJson(const json_spirit::Object& obj);
};

struct AccountIdentifier {
    std::string address;
    
    json_spirit::Object ToJson() const;
    static AccountIdentifier FromJson(const json_spirit::Object& obj);
};

struct TransactionIdentifier {
    std::string hash;
    
    json_spirit::Object ToJson() const;
    static TransactionIdentifier FromJson(const json_spirit::Object& obj);
};

struct Currency {
    std::string symbol;
    int decimals;
    std::string metadata;
    
    json_spirit::Object ToJson() const;
    static Currency FromJson(const json_spirit::Object& obj);
};

struct Amount {
    std::string value;
    Currency currency;
    
    json_spirit::Object ToJson() const;
    static Amount FromJson(const json_spirit::Object& obj);
};

struct PartialBlockIdentifier {
    uint64_t index;
    std::string hash;
    
    json_spirit::Object ToJson() const;
    static PartialBlockIdentifier FromJson(const json_spirit::Object& obj);
};

// Initialize and start the Rosetta API server
bool StartRosettaServer();

// Shutdown the Rosetta API server
void ShutdownRosettaServer();

} // namespace Rosetta

#endif // IOCOIN_ROSETTA_H
