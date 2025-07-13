// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rosetta.h"
#include "../util.h"
#include "../main.h"
#include "../init.h"
#include "../wallet.h"
#include "../base58.h"
#include "../sync.h"

namespace Rosetta {

// Implementation of the JSON conversion methods for the Rosetta types

json_spirit::Object NetworkIdentifier::ToJson() const {
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("blockchain", blockchain));
    obj.push_back(json_spirit::Pair("network", network));
    if (!sub_network.empty())
        obj.push_back(json_spirit::Pair("sub_network_identifier", sub_network));
    return obj;
}

NetworkIdentifier NetworkIdentifier::FromJson(const json_spirit::Object& obj) {
    NetworkIdentifier result;
    result.blockchain = find_value(obj, "blockchain").get_str();
    result.network = find_value(obj, "network").get_str();
    json_spirit::Value sub_network = find_value(obj, "sub_network_identifier");
    if (sub_network.type() != json_spirit::null_type) {
        result.sub_network = sub_network.get_str();
    }
    return result;
}

json_spirit::Object BlockIdentifier::ToJson() const {
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("index", (uint64_t)index));
    obj.push_back(json_spirit::Pair("hash", hash));
    return obj;
}

BlockIdentifier BlockIdentifier::FromJson(const json_spirit::Object& obj) {
    BlockIdentifier result;
    result.index = find_value(obj, "index").get_uint64();
    result.hash = find_value(obj, "hash").get_str();
    return result;
}

json_spirit::Object AccountIdentifier::ToJson() const {
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("address", address));
    return obj;
}

AccountIdentifier AccountIdentifier::FromJson(const json_spirit::Object& obj) {
    AccountIdentifier result;
    result.address = find_value(obj, "address").get_str();
    return result;
}

json_spirit::Object TransactionIdentifier::ToJson() const {
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("hash", hash));
    return obj;
}

TransactionIdentifier TransactionIdentifier::FromJson(const json_spirit::Object& obj) {
    TransactionIdentifier result;
    result.hash = find_value(obj, "hash").get_str();
    return result;
}

json_spirit::Object Currency::ToJson() const {
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("symbol", symbol));
    obj.push_back(json_spirit::Pair("decimals", decimals));
    if (!metadata.empty())
        obj.push_back(json_spirit::Pair("metadata", metadata));
    return obj;
}

Currency Currency::FromJson(const json_spirit::Object& obj) {
    Currency result;
    result.symbol = find_value(obj, "symbol").get_str();
    result.decimals = find_value(obj, "decimals").get_int();
    json_spirit::Value metadata = find_value(obj, "metadata");
    if (metadata.type() != json_spirit::null_type) {
        result.metadata = metadata.get_str();
    }
    return result;
}

json_spirit::Object Amount::ToJson() const {
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("value", value));
    obj.push_back(json_spirit::Pair("currency", currency.ToJson()));
    return obj;
}

Amount Amount::FromJson(const json_spirit::Object& obj) {
    Amount result;
    result.value = find_value(obj, "value").get_str();
    json_spirit::Object currencyObj = find_value(obj, "currency").get_obj();
    result.currency = Currency::FromJson(currencyObj);
    return result;
}

json_spirit::Object PartialBlockIdentifier::ToJson() const {
    json_spirit::Object obj;
    if (index != 0)
        obj.push_back(json_spirit::Pair("index", (uint64_t)index));
    if (!hash.empty())
        obj.push_back(json_spirit::Pair("hash", hash));
    return obj;
}

PartialBlockIdentifier PartialBlockIdentifier::FromJson(const json_spirit::Object& obj) {
    PartialBlockIdentifier result;
    json_spirit::Value indexVal = find_value(obj, "index");
    if (indexVal.type() != json_spirit::null_type) {
        result.index = indexVal.get_uint64();
    } else {
        result.index = 0;
    }
    
    json_spirit::Value hashVal = find_value(obj, "hash");
    if (hashVal.type() != json_spirit::null_type) {
        result.hash = hashVal.get_str();
    } else {
        result.hash = "";
    }
    return result;
}

// Rosetta Server Implementation
class ServerImpl {
public:
    ServerImpl();
    ~ServerImpl();

    bool Start();
    void Shutdown();
    
    // Check if the server is running
    bool IsRunning() const;

private:
    // Server state
    bool running;
};

ServerImpl::ServerImpl() : running(false) {
}

ServerImpl::~ServerImpl() {
    if (running) {
        Shutdown();
    }
}

bool ServerImpl::Start() {
    if (running) {
        return true;
    }
    
    // Initialize the server and start listening
    // This is a placeholder - actual implementation will depend on the HTTP server library used
    LogPrintf("Starting Rosetta API server on port %d\n", GetArg("-rosettaport", 8080));
    running = true;
    
    return true;
}

void ServerImpl::Shutdown() {
    if (!running) {
        return;
    }
    
    // Shutdown the server
    LogPrintf("Shutting down Rosetta API server\n");
    running = false;
}

bool ServerImpl::IsRunning() const {
    return running;
}

// Global server instance
static std::unique_ptr<ServerImpl> g_rosettaServer;

bool StartRosettaServer() {
    if (g_rosettaServer) {
        return g_rosettaServer->IsRunning();
    }
    
    g_rosettaServer.reset(new ServerImpl());
    return g_rosettaServer->Start();
}

void ShutdownRosettaServer() {
    if (g_rosettaServer) {
        g_rosettaServer->Shutdown();
        g_rosettaServer.reset();
    }
}

} // namespace Rosetta
