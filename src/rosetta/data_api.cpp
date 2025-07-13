// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "data_api.h"
#include "../main.h"
#include "../wallet.h"
#include "../db.h"
#include "../bitcoinrpc.h"
#include "../util.h"
#include "../base58.h"

namespace Rosetta {
namespace DataAPI {

Response NetworkList(const json_spirit::Object& request) {
    Response response;
    
    json_spirit::Array network_identifiers;
    
    // Add mainnet
    json_spirit::Object mainnet;
    mainnet.push_back(json_spirit::Pair("blockchain", "IOCoin"));
    mainnet.push_back(json_spirit::Pair("network", "mainnet"));
    network_identifiers.push_back(mainnet);
    
    // Add testnet if applicable
    if (fTestNet) {
        json_spirit::Object testnet;
        testnet.push_back(json_spirit::Pair("blockchain", "IOCoin"));
        testnet.push_back(json_spirit::Pair("network", "testnet"));
        network_identifiers.push_back(testnet);
    }
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("network_identifiers", network_identifiers));
    
    response.result = result;
    return response;
}

Response NetworkStatus(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    json_spirit::Object network = network_identifier.get_obj();
    std::string blockchain = find_value(network, "blockchain").get_str();
    std::string network_name = find_value(network, "network").get_str();
    
    // Validate the network is supported
    if (blockchain != "IOCoin" || 
        (network_name != "mainnet" && network_name != "testnet") || 
        (network_name == "testnet" && !fTestNet) || 
        (network_name == "mainnet" && fTestNet)) {
        response.code = BadRequest;
        response.message = "Unsupported network";
        return response;
    }
    
    // Get the current block height
    int current_block_height = nBestHeight;
    
    // Get the current block hash
    uint256 current_block_hash = pindexBest->GetBlockHash();
    
    // Get the genesis block hash
    uint256 genesis_block_hash = hashGenesisBlock;
    
    json_spirit::Object current_block_identifier;
    current_block_identifier.push_back(json_spirit::Pair("index", current_block_height));
    current_block_identifier.push_back(json_spirit::Pair("hash", current_block_hash.GetHex()));
    
    json_spirit::Object genesis_block_identifier;
    genesis_block_identifier.push_back(json_spirit::Pair("index", 0));
    genesis_block_identifier.push_back(json_spirit::Pair("hash", genesis_block_hash.GetHex()));
    
    // Get the current block timestamp
    int64_t current_block_timestamp = pindexBest->GetBlockTime() * 1000; // Convert to milliseconds
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("current_block_identifier", current_block_identifier));
    result.push_back(json_spirit::Pair("current_block_timestamp", current_block_timestamp));
    result.push_back(json_spirit::Pair("genesis_block_identifier", genesis_block_identifier));
    
    // Add peers information if available
    // This is optional in Rosetta spec
    
    response.result = result;
    return response;
}

Response NetworkOptions(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    json_spirit::Object network = network_identifier.get_obj();
    std::string blockchain = find_value(network, "blockchain").get_str();
    std::string network_name = find_value(network, "network").get_str();
    
    // Validate the network is supported
    if (blockchain != "IOCoin" || 
        (network_name != "mainnet" && network_name != "testnet") || 
        (network_name == "testnet" && !fTestNet) || 
        (network_name == "mainnet" && fTestNet)) {
        response.code = BadRequest;
        response.message = "Unsupported network";
        return response;
    }
    
    json_spirit::Object version;
    version.push_back(json_spirit::Pair("rosetta_version", ROSETTA_API_VERSION));
    version.push_back(json_spirit::Pair("node_version", FormatFullVersion()));
    
    json_spirit::Array operation_statuses;
    
    json_spirit::Object success_status;
    success_status.push_back(json_spirit::Pair("status", "SUCCESS"));
    success_status.push_back(json_spirit::Pair("successful", true));
    operation_statuses.push_back(success_status);
    
    json_spirit::Object failed_status;
    failed_status.push_back(json_spirit::Pair("status", "FAILED"));
    failed_status.push_back(json_spirit::Pair("successful", false));
    operation_statuses.push_back(failed_status);
    
    json_spirit::Array operation_types;
    operation_types.push_back("TRANSFER");
    operation_types.push_back("COINBASE");
    operation_types.push_back("FEE");
    
    json_spirit::Array errors;
    
    // Define standard errors
    json_spirit::Object invalid_network;
    invalid_network.push_back(json_spirit::Pair("code", InvalidNetworkIdentifier));
    invalid_network.push_back(json_spirit::Pair("message", "Invalid network identifier"));
    errors.push_back(invalid_network);
    
    json_spirit::Object invalid_account;
    invalid_account.push_back(json_spirit::Pair("code", InvalidAccountIdentifier));
    invalid_account.push_back(json_spirit::Pair("message", "Invalid account identifier"));
    errors.push_back(invalid_account);
    
    json_spirit::Object invalid_block;
    invalid_block.push_back(json_spirit::Pair("code", InvalidBlockIdentifier));
    invalid_block.push_back(json_spirit::Pair("message", "Invalid block identifier"));
    errors.push_back(invalid_block);
    
    json_spirit::Object invalid_transaction;
    invalid_transaction.push_back(json_spirit::Pair("code", InvalidTransactionIdentifier));
    invalid_transaction.push_back(json_spirit::Pair("message", "Invalid transaction identifier"));
    errors.push_back(invalid_transaction);
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("version", version));
    result.push_back(json_spirit::Pair("operation_statuses", operation_statuses));
    result.push_back(json_spirit::Pair("operation_types", operation_types));
    result.push_back(json_spirit::Pair("errors", errors));
    
    // Add currencies
    json_spirit::Array currencies;
    json_spirit::Object ioc_currency;
    ioc_currency.push_back(json_spirit::Pair("symbol", "IOC"));
    ioc_currency.push_back(json_spirit::Pair("decimals", 8));
    currencies.push_back(ioc_currency);
    result.push_back(json_spirit::Pair("allow_historical_balance_lookup", true));
    result.push_back(json_spirit::Pair("currencies", currencies));
    
    response.result = result;
    return response;
}

Response AccountBalance(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    // Validate the account identifier in the request
    json_spirit::Value account_identifier = find_value(request, "account_identifier");
    if (account_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid account_identifier";
        return response;
    }
    
    json_spirit::Object account = account_identifier.get_obj();
    std::string address = find_value(account, "address").get_str();
    
    // Validate the address is a valid IOCoin address
    CBitcoinAddress bitcoin_address(address);
    if (!bitcoin_address.IsValid()) {
        response.code = InvalidAccountIdentifier;
        response.message = "Invalid IOCoin address";
        return response;
    }
    
    // Check if we have the block identifier for historical balance lookup
    CBlockIndex* block_index = pindexBest;
    json_spirit::Value block_identifier = find_value(request, "block_identifier");
    if (block_identifier.type() == json_spirit::obj_type) {
        json_spirit::Object block = block_identifier.get_obj();
        
        json_spirit::Value index_value = find_value(block, "index");
        json_spirit::Value hash_value = find_value(block, "hash");
        
        if (hash_value.type() == json_spirit::str_type) {
            uint256 block_hash;
            block_hash.SetHex(hash_value.get_str());
            std::map<uint256, CBlockIndex*>::iterator it = mapBlockIndex.find(block_hash);
            if (it == mapBlockIndex.end()) {
                response.code = InvalidBlockIdentifier;
                response.message = "Block not found";
                return response;
            }
            block_index = it->second;
        } else if (index_value.type() == json_spirit::int_type) {
            int block_height = index_value.get_int();
            if (block_height > nBestHeight) {
                response.code = InvalidBlockIdentifier;
                response.message = "Block height is greater than current height";
                return response;
            }
            
            CBlockIndex* pindex = pindexBest;
            while (pindex && pindex->nHeight > block_height) {
                pindex = pindex->pprev;
            }
            
            if (!pindex || pindex->nHeight != block_height) {
                response.code = InvalidBlockIdentifier;
                response.message = "Block not found";
                return response;
            }
            
            block_index = pindex;
        }
    }
    
    // Get the balance for the address
    // Note: This is a simplified version, as IOCoin may not have direct address balance lookup
    // A more complete implementation would scan the UTXO set for outputs to this address
    
    int64_t balance = 0;
    
    // For demonstration purposes, if this is the wallet's own address, we can use the wallet balance
    if (pwalletMain->HaveKey(bitcoin_address.Get())) {
        balance = pwalletMain->GetBalance();
    } else {
        // For non-wallet addresses, we would need to scan the UTXO set
        // This is just a placeholder - actual implementation would be more complex
        balance = 0;
    }
    
    json_spirit::Object block_identifier_obj;
    block_identifier_obj.push_back(json_spirit::Pair("index", block_index->nHeight));
    block_identifier_obj.push_back(json_spirit::Pair("hash", block_index->GetBlockHash().GetHex()));
    
    json_spirit::Array balances;
    json_spirit::Object ioc_balance;
    ioc_balance.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(balance)));
    
    json_spirit::Object currency;
    currency.push_back(json_spirit::Pair("symbol", "IOC"));
    currency.push_back(json_spirit::Pair("decimals", 8));
    ioc_balance.push_back(json_spirit::Pair("currency", currency));
    
    balances.push_back(ioc_balance);
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("block_identifier", block_identifier_obj));
    result.push_back(json_spirit::Pair("balances", balances));
    
    response.result = result;
    return response;
}

Response Block(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    // Validate the block identifier in the request
    json_spirit::Value block_identifier = find_value(request, "block_identifier");
    if (block_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid block_identifier";
        return response;
    }
    
    json_spirit::Object block = block_identifier.get_obj();
    
    CBlockIndex* block_index = NULL;
    json_spirit::Value index_value = find_value(block, "index");
    json_spirit::Value hash_value = find_value(block, "hash");
    
    if (hash_value.type() == json_spirit::str_type) {
        uint256 block_hash;
        block_hash.SetHex(hash_value.get_str());
        std::map<uint256, CBlockIndex*>::iterator it = mapBlockIndex.find(block_hash);
        if (it == mapBlockIndex.end()) {
            response.code = InvalidBlockIdentifier;
            response.message = "Block not found";
            return response;
        }
        block_index = it->second;
    } else if (index_value.type() == json_spirit::int_type) {
        int block_height = index_value.get_int();
        if (block_height > nBestHeight) {
            response.code = InvalidBlockIdentifier;
            response.message = "Block height is greater than current height";
            return response;
        }
        
        CBlockIndex* pindex = pindexBest;
        while (pindex && pindex->nHeight > block_height) {
            pindex = pindex->pprev;
        }
        
        if (!pindex || pindex->nHeight != block_height) {
            response.code = InvalidBlockIdentifier;
            response.message = "Block not found";
            return response;
        }
        
        block_index = pindex;
    } else {
        response.code = BadRequest;
        response.message = "Block identifier must include either index or hash";
        return response;
    }
    
    // Read the block from disk
    CBlock block_data;
    if (!block_data.ReadFromDisk(block_index, true)) {
        response.code = ServerError;
        response.message = "Could not read block from disk";
        return response;
    }
    
    // Create the response
    json_spirit::Object block_identifier_obj;
    block_identifier_obj.push_back(json_spirit::Pair("index", block_index->nHeight));
    block_identifier_obj.push_back(json_spirit::Pair("hash", block_index->GetBlockHash().GetHex()));
    
    json_spirit::Object parent_block_identifier;
    if (block_index->pprev) {
        parent_block_identifier.push_back(json_spirit::Pair("index", block_index->pprev->nHeight));
        parent_block_identifier.push_back(json_spirit::Pair("hash", block_index->pprev->GetBlockHash().GetHex()));
    } else {
        // Genesis block case
        parent_block_identifier.push_back(json_spirit::Pair("index", 0));
        parent_block_identifier.push_back(json_spirit::Pair("hash", block_index->GetBlockHash().GetHex()));
    }
    
    int64_t timestamp = block_index->GetBlockTime() * 1000; // Convert to milliseconds
    
    json_spirit::Array transactions;
    BOOST_FOREACH(const CTransaction& tx, block_data.vtx) {
        json_spirit::Object transaction;
        json_spirit::Object transaction_identifier;
        transaction_identifier.push_back(json_spirit::Pair("hash", tx.GetHash().GetHex()));
        transaction.push_back(json_spirit::Pair("transaction_identifier", transaction_identifier));
        
        // Build the operations (inputs and outputs)
        json_spirit::Array operations;
        int op_index = 0;
        
        // Handle inputs
        if (!tx.IsCoinBase()) {
            for (unsigned int i = 0; i < tx.vin.size(); i++) {
                const CTxIn& txin = tx.vin[i];
                
                // Get the previous transaction output
                CTransaction prev_tx;
                uint256 hash_block;
                if (!GetTransaction(txin.prevout.hash, prev_tx, hash_block)) {
                    // Skip if we can't find the previous transaction
                    continue;
                }
                
                if (txin.prevout.n >= prev_tx.vout.size()) {
                    // Skip if the output index is out of range
                    continue;
                }
                
                const CTxOut& prev_out = prev_tx.vout[txin.prevout.n];
                
                // Get the address from the scriptPubKey
                CTxDestination dest;
                if (!ExtractDestination(prev_out.scriptPubKey, dest)) {
                    // Skip if we can't extract the destination
                    continue;
                }
                
                json_spirit::Object operation;
                operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
                operation.push_back(json_spirit::Pair("type", "TRANSFER"));
                operation.push_back(json_spirit::Pair("status", "SUCCESS"));
                
                json_spirit::Object account;
                account.push_back(json_spirit::Pair("address", CBitcoinAddress(dest).ToString()));
                operation.push_back(json_spirit::Pair("account", account));
                
                json_spirit::Object amount;
                amount.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(-prev_out.nValue)));
                
                json_spirit::Object currency;
                currency.push_back(json_spirit::Pair("symbol", "IOC"));
                currency.push_back(json_spirit::Pair("decimals", 8));
                amount.push_back(json_spirit::Pair("currency", currency));
                
                operation.push_back(json_spirit::Pair("amount", amount));
                operations.push_back(operation);
            }
        } else {
            // Coinbase transaction
            json_spirit::Object operation;
            operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
            operation.push_back(json_spirit::Pair("type", "COINBASE"));
            operation.push_back(json_spirit::Pair("status", "SUCCESS"));
            operations.push_back(operation);
        }
        
        // Handle outputs
        for (unsigned int i = 0; i < tx.vout.size(); i++) {
            const CTxOut& txout = tx.vout[i];
            
            // Get the address from the scriptPubKey
            CTxDestination dest;
            if (!ExtractDestination(txout.scriptPubKey, dest)) {
                // Skip if we can't extract the destination
                continue;
            }
            
            json_spirit::Object operation;
            operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
            operation.push_back(json_spirit::Pair("type", "TRANSFER"));
            operation.push_back(json_spirit::Pair("status", "SUCCESS"));
            
            json_spirit::Object account;
            account.push_back(json_spirit::Pair("address", CBitcoinAddress(dest).ToString()));
            operation.push_back(json_spirit::Pair("account", account));
            
            json_spirit::Object amount;
            amount.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(txout.nValue)));
            
            json_spirit::Object currency;
            currency.push_back(json_spirit::Pair("symbol", "IOC"));
            currency.push_back(json_spirit::Pair("decimals", 8));
            amount.push_back(json_spirit::Pair("currency", currency));
            
            operation.push_back(json_spirit::Pair("amount", amount));
            operations.push_back(operation);
        }
        
        transaction.push_back(json_spirit::Pair("operations", operations));
        transactions.push_back(transaction);
    }
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("block", json_spirit::Object({
        {"block_identifier", block_identifier_obj},
        {"parent_block_identifier", parent_block_identifier},
        {"timestamp", timestamp},
        {"transactions", transactions}
    })));
    
    response.result = result;
    return response;
}

Response BlockTransaction(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    // Validate the block identifier in the request
    json_spirit::Value block_identifier = find_value(request, "block_identifier");
    if (block_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid block_identifier";
        return response;
    }
    
    // Validate the transaction identifier in the request
    json_spirit::Value transaction_identifier = find_value(request, "transaction_identifier");
    if (transaction_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid transaction_identifier";
        return response;
    }
    
    json_spirit::Object tx_id = transaction_identifier.get_obj();
    std::string tx_hash_str = find_value(tx_id, "hash").get_str();
    uint256 tx_hash;
    tx_hash.SetHex(tx_hash_str);
    
    json_spirit::Object block = block_identifier.get_obj();
    uint256 block_hash;
    
    json_spirit::Value hash_value = find_value(block, "hash");
    if (hash_value.type() == json_spirit::str_type) {
        block_hash.SetHex(hash_value.get_str());
    } else {
        response.code = BadRequest;
        response.message = "Block hash is required";
        return response;
    }
    
    // Check if block exists
    std::map<uint256, CBlockIndex*>::iterator it = mapBlockIndex.find(block_hash);
    if (it == mapBlockIndex.end()) {
        response.code = InvalidBlockIdentifier;
        response.message = "Block not found";
        return response;
    }
    
    CBlockIndex* block_index = it->second;
    
    // Read the block from disk
    CBlock block_data;
    if (!block_data.ReadFromDisk(block_index, true)) {
        response.code = ServerError;
        response.message = "Could not read block from disk";
        return response;
    }
    
    // Find the transaction in the block
    CTransaction found_tx;
    bool found = false;
    
    BOOST_FOREACH(const CTransaction& tx, block_data.vtx) {
        if (tx.GetHash() == tx_hash) {
            found_tx = tx;
            found = true;
            break;
        }
    }
    
    if (!found) {
        response.code = InvalidTransactionIdentifier;
        response.message = "Transaction not found in block";
        return response;
    }
    
    // Create the response
    json_spirit::Object transaction_identifier_obj;
    transaction_identifier_obj.push_back(json_spirit::Pair("hash", tx_hash.GetHex()));
    
    json_spirit::Array operations;
    int op_index = 0;
    
    // Handle inputs
    if (!found_tx.IsCoinBase()) {
        for (unsigned int i = 0; i < found_tx.vin.size(); i++) {
            const CTxIn& txin = found_tx.vin[i];
            
            // Get the previous transaction output
            CTransaction prev_tx;
            uint256 hash_block;
            if (!GetTransaction(txin.prevout.hash, prev_tx, hash_block)) {
                // Skip if we can't find the previous transaction
                continue;
            }
            
            if (txin.prevout.n >= prev_tx.vout.size()) {
                // Skip if the output index is out of range
                continue;
            }
            
            const CTxOut& prev_out = prev_tx.vout[txin.prevout.n];
            
            // Get the address from the scriptPubKey
            CTxDestination dest;
            if (!ExtractDestination(prev_out.scriptPubKey, dest)) {
                // Skip if we can't extract the destination
                continue;
            }
            
            json_spirit::Object operation;
            operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
            operation.push_back(json_spirit::Pair("type", "TRANSFER"));
            operation.push_back(json_spirit::Pair("status", "SUCCESS"));
            
            json_spirit::Object account;
            account.push_back(json_spirit::Pair("address", CBitcoinAddress(dest).ToString()));
            operation.push_back(json_spirit::Pair("account", account));
            
            json_spirit::Object amount;
            amount.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(-prev_out.nValue)));
            
            json_spirit::Object currency;
            currency.push_back(json_spirit::Pair("symbol", "IOC"));
            currency.push_back(json_spirit::Pair("decimals", 8));
            amount.push_back(json_spirit::Pair("currency", currency));
            
            operation.push_back(json_spirit::Pair("amount", amount));
            operations.push_back(operation);
        }
    } else {
        // Coinbase transaction
        json_spirit::Object operation;
        operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
        operation.push_back(json_spirit::Pair("type", "COINBASE"));
        operation.push_back(json_spirit::Pair("status", "SUCCESS"));
        operations.push_back(operation);
    }
    
    // Handle outputs
    for (unsigned int i = 0; i < found_tx.vout.size(); i++) {
        const CTxOut& txout = found_tx.vout[i];
        
        // Get the address from the scriptPubKey
        CTxDestination dest;
        if (!ExtractDestination(txout.scriptPubKey, dest)) {
            // Skip if we can't extract the destination
            continue;
        }
        
        json_spirit::Object operation;
        operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
        operation.push_back(json_spirit::Pair("type", "TRANSFER"));
        operation.push_back(json_spirit::Pair("status", "SUCCESS"));
        
        json_spirit::Object account;
        account.push_back(json_spirit::Pair("address", CBitcoinAddress(dest).ToString()));
        operation.push_back(json_spirit::Pair("account", account));
        
        json_spirit::Object amount;
        amount.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(txout.nValue)));
        
        json_spirit::Object currency;
        currency.push_back(json_spirit::Pair("symbol", "IOC"));
        currency.push_back(json_spirit::Pair("decimals", 8));
        amount.push_back(json_spirit::Pair("currency", currency));
        
        operation.push_back(json_spirit::Pair("amount", amount));
        operations.push_back(operation);
    }
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("transaction", json_spirit::Object({
        {"transaction_identifier", transaction_identifier_obj},
        {"operations", operations}
    })));
    
    response.result = result;
    return response;
}

Response Mempool(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    // Get the mempool transactions
    std::vector<uint256> mempool_hashes = mempool.queryHashes();
    
    json_spirit::Array transaction_identifiers;
    BOOST_FOREACH(const uint256& hash, mempool_hashes) {
        json_spirit::Object tx_id;
        tx_id.push_back(json_spirit::Pair("hash", hash.GetHex()));
        transaction_identifiers.push_back(tx_id);
    }
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("transaction_identifiers", transaction_identifiers));
    
    response.result = result;
    return response;
}

Response MempoolTransaction(const json_spirit::Object& request) {
    Response response;
    
    // Validate the network identifier in the request
    json_spirit::Value network_identifier = find_value(request, "network_identifier");
    if (network_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid network_identifier";
        return response;
    }
    
    // Validate the transaction identifier in the request
    json_spirit::Value transaction_identifier = find_value(request, "transaction_identifier");
    if (transaction_identifier.type() != json_spirit::obj_type) {
        response.code = BadRequest;
        response.message = "Missing or invalid transaction_identifier";
        return response;
    }
    
    json_spirit::Object tx_id = transaction_identifier.get_obj();
    std::string tx_hash_str = find_value(tx_id, "hash").get_str();
    uint256 tx_hash;
    tx_hash.SetHex(tx_hash_str);
    
    // Check if the transaction is in the mempool
    if (!mempool.exists(tx_hash)) {
        response.code = InvalidTransactionIdentifier;
        response.message = "Transaction not found in mempool";
        return response;
    }
    
    // Get the transaction from mempool
    CTransaction tx;
    if (!mempool.lookup(tx_hash, tx)) {
        response.code = ServerError;
        response.message = "Failed to get transaction from mempool";
        return response;
    }
    
    // Create the response
    json_spirit::Object transaction_identifier_obj;
    transaction_identifier_obj.push_back(json_spirit::Pair("hash", tx_hash.GetHex()));
    
    json_spirit::Array operations;
    int op_index = 0;
    
    // Handle inputs
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        const CTxIn& txin = tx.vin[i];
        
        // Get the previous transaction output
        CTransaction prev_tx;
        uint256 hash_block;
        if (!GetTransaction(txin.prevout.hash, prev_tx, hash_block)) {
            // Skip if we can't find the previous transaction
            continue;
        }
        
        if (txin.prevout.n >= prev_tx.vout.size()) {
            // Skip if the output index is out of range
            continue;
        }
        
        const CTxOut& prev_out = prev_tx.vout[txin.prevout.n];
        
        // Get the address from the scriptPubKey
        CTxDestination dest;
        if (!ExtractDestination(prev_out.scriptPubKey, dest)) {
            // Skip if we can't extract the destination
            continue;
        }
        
        json_spirit::Object operation;
        operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
        operation.push_back(json_spirit::Pair("type", "TRANSFER"));
        operation.push_back(json_spirit::Pair("status", "SUCCESS"));
        
        json_spirit::Object account;
        account.push_back(json_spirit::Pair("address", CBitcoinAddress(dest).ToString()));
        operation.push_back(json_spirit::Pair("account", account));
        
        json_spirit::Object amount;
        amount.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(-prev_out.nValue)));
        
        json_spirit::Object currency;
        currency.push_back(json_spirit::Pair("symbol", "IOC"));
        currency.push_back(json_spirit::Pair("decimals", 8));
        amount.push_back(json_spirit::Pair("currency", currency));
        
        operation.push_back(json_spirit::Pair("amount", amount));
        operations.push_back(operation);
    }
    
    // Handle outputs
    for (unsigned int i = 0; i < tx.vout.size(); i++) {
        const CTxOut& txout = tx.vout[i];
        
        // Get the address from the scriptPubKey
        CTxDestination dest;
        if (!ExtractDestination(txout.scriptPubKey, dest)) {
            // Skip if we can't extract the destination
            continue;
        }
        
        json_spirit::Object operation;
        operation.push_back(json_spirit::Pair("operation_identifier", json_spirit::Object({{"index", op_index++}})));
        operation.push_back(json_spirit::Pair("type", "TRANSFER"));
        operation.push_back(json_spirit::Pair("status", "SUCCESS"));
        
        json_spirit::Object account;
        account.push_back(json_spirit::Pair("address", CBitcoinAddress(dest).ToString()));
        operation.push_back(json_spirit::Pair("account", account));
        
        json_spirit::Object amount;
        amount.push_back(json_spirit::Pair("value", boost::lexical_cast<std::string>(txout.nValue)));
        
        json_spirit::Object currency;
        currency.push_back(json_spirit::Pair("symbol", "IOC"));
        currency.push_back(json_spirit::Pair("decimals", 8));
        amount.push_back(json_spirit::Pair("currency", currency));
        
        operation.push_back(json_spirit::Pair("amount", amount));
        operations.push_back(operation);
    }
    
    json_spirit::Object result;
    result.push_back(json_spirit::Pair("transaction", json_spirit::Object({
        {"transaction_identifier", transaction_identifier_obj},
        {"operations", operations}
    })));
    
    response.result = result;
    return response;
}

} // namespace DataAPI
} // namespace Rosetta
