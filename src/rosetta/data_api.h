// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IOCOIN_ROSETTA_DATA_API_H
#define IOCOIN_ROSETTA_DATA_API_H

#include "rosetta.h"

namespace Rosetta {
namespace DataAPI {

// Data API endpoints

// /network/list endpoint
// Returns a list of NetworkIdentifiers that the Rosetta implementation supports
Response NetworkList(const json_spirit::Object& request);

// /network/status endpoint
// Returns the current status of the network requested
Response NetworkStatus(const json_spirit::Object& request);

// /network/options endpoint
// Returns the version information and allowed network-specific types for a NetworkIdentifier
Response NetworkOptions(const json_spirit::Object& request);

// /account/balance endpoint
// Returns the balance of an AccountIdentifier at a block height
Response AccountBalance(const json_spirit::Object& request);

// /block endpoint
// Returns a block and all its transactions
Response Block(const json_spirit::Object& request);

// /block/transaction endpoint
// Returns a transaction within a block
Response BlockTransaction(const json_spirit::Object& request);

// /mempool endpoint
// Returns a list of transaction identifiers in the mempool
Response Mempool(const json_spirit::Object& request);

// /mempool/transaction endpoint
// Returns a transaction from the mempool
Response MempoolTransaction(const json_spirit::Object& request);

} // namespace DataAPI
} // namespace Rosetta

#endif // IOCOIN_ROSETTA_DATA_API_H
