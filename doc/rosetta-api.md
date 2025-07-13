# IOCoin Rosetta API Implementation

This document describes the implementation of the Rosetta API for IOCoin, which enables IOCoin to be compatible with the Coinbase integration requirements.

## Overview

The Rosetta API is a specification developed by Coinbase to standardize how blockchains interact with exchanges. This implementation follows version 1.4.13 of the Rosetta specification.

## Architecture

The IOCoin Rosetta API implementation consists of the following components:

1. **Core Rosetta Types** - Defined in `rosetta.h` and implemented in `rosetta.cpp`
2. **Data API** - Implemented in `data_api.h/cpp` for blockchain data queries
3. **Construction API** - Implemented in `construction_api.h/cpp` for transaction construction
4. **HTTP Server** - Implemented in `rosetta_server.h/cpp` to handle HTTP requests
5. **Thread Management** - Implemented in `rosetta_main.cpp` to manage the server thread

## Endpoints

### Data API

The following Data API endpoints are implemented:

- `/network/list` - Returns a list of available networks (mainnet/testnet)
- `/network/status` - Returns the current status of a network
- `/network/options` - Returns the version information and allowed types
- `/account/balance` - Returns the balance of an account
- `/block` - Returns a block and all its transactions
- `/block/transaction` - Returns a transaction within a block
- `/mempool` - Returns a list of transaction identifiers in the mempool
- `/mempool/transaction` - Returns a transaction from the mempool

### Construction API

The following Construction API endpoints are implemented:

- `/construction/derive` - Derives an address from a public key
- `/construction/preprocess` - Creates a request to fetch metadata needed for construction
- `/construction/metadata` - Returns metadata needed for transaction construction
- `/construction/payloads` - Creates an unsigned transaction blob and payloads to sign
- `/construction/parse` - Parses a transaction and returns operations and signers
- `/construction/combine` - Creates a network transaction from an unsigned transaction and signatures
- `/construction/hash` - Returns the transaction hash for a signed transaction
- `/construction/submit` - Submits a signed transaction to the network

## Configuration

To enable the Rosetta API, the following command-line parameters are available:

- `-rosetta` - Enable the Rosetta API (default: 0)
- `-rosettaport=<n>` - Listen for Rosetta API connections on port `<n>` (default: 8080)

Example:
```
./iocoind -rosetta -rosettaport=8080
```

## Testing

To test the IOCoin Rosetta API implementation:

1. Install the `rosetta-cli` tool from [https://github.com/coinbase/rosetta-cli](https://github.com/coinbase/rosetta-cli)
2. Run the IOCoin daemon with Rosetta API enabled: `./iocoind -rosetta`
3. Run the data validation: `rosetta-cli check:data --configuration-file rosetta-cli-conf.json`
4. Run the construction validation (if implemented): `rosetta-cli check:construction --configuration-file rosetta-cli-conf.json`

## Integration with Coinbase

For Coinbase integration, follow these steps:

1. Ensure the IOCoin node is running with the Rosetta API enabled
2. Configure the node to be accessible to Coinbase's systems
3. Provide Coinbase with the endpoint information
4. Work with Coinbase to validate the implementation

## Limitations and Future Work

Current limitations:

1. Construction API is implemented as a placeholder - full implementation requires more comprehensive integration with wallet functionality
2. Account balance retrieval currently only supports wallet-owned addresses
3. Full UTXO scanning for non-wallet addresses is not implemented yet

Future work:

1. Complete Construction API implementation
2. Implement full UTXO scanning for any address balance
3. Add comprehensive test coverage
4. Optimize performance for high-load scenarios
