// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IOCOIN_ROSETTA_CONSTRUCTION_API_H
#define IOCOIN_ROSETTA_CONSTRUCTION_API_H

#include "rosetta.h"

namespace Rosetta {
namespace ConstructionAPI {

// Construction API endpoints

// /construction/derive endpoint
// Derives an address from a public key
Response Derive(const json_spirit::Object& request);

// /construction/preprocess endpoint
// Creates a request to fetch metadata needed for construction
Response Preprocess(const json_spirit::Object& request);

// /construction/metadata endpoint
// Returns metadata needed for transaction construction
Response Metadata(const json_spirit::Object& request);

// /construction/payloads endpoint
// Creates an unsigned transaction blob and a collection of payloads that must be signed
Response Payloads(const json_spirit::Object& request);

// /construction/parse endpoint
// Parses a transaction and returns operations and signers
Response Parse(const json_spirit::Object& request);

// /construction/combine endpoint
// Creates a network transaction from an unsigned transaction and signatures
Response Combine(const json_spirit::Object& request);

// /construction/hash endpoint
// Returns the transaction hash for a signed transaction
Response Hash(const json_spirit::Object& request);

// /construction/submit endpoint
// Submits a signed transaction to the network
Response Submit(const json_spirit::Object& request);

} // namespace ConstructionAPI
} // namespace Rosetta

#endif // IOCOIN_ROSETTA_CONSTRUCTION_API_H
