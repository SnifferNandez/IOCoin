// Copyright (c) 2023-2025 IOCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rosetta.h"
#include "rosetta_server.h"
#include "data_api.h"
#include "construction_api.h"
#include "../util.h"
#include "../init.h"

#include <memory>

namespace Rosetta {

// Global server instance
static std::unique_ptr<RosettaServer> g_rosettaServer;

bool StartRosettaServer() {
    // Check if server is already running
    if (g_rosettaServer && g_rosettaServer->IsRunning()) {
        return true;
    }
    
    // Get the port from the configuration
    uint16_t port = (uint16_t)GetArg("-rosettaport", 8080);
    
    // Create and start the server
    g_rosettaServer.reset(new RosettaServer(port));
    return g_rosettaServer->Start();
}

void ShutdownRosettaServer() {
    if (g_rosettaServer) {
        g_rosettaServer->Stop();
        g_rosettaServer.reset();
    }
}

void ThreadRosettaServer(void* parg) {
    // Name this thread
    RenameThread("iocoin-rosetta");
    
    try {
        // Start the Rosetta API server
        LogPrintf("Starting Rosetta API thread\n");
        StartRosettaServer();
        
        // Keep thread alive
        while (1) {
            // Sleep for 5 seconds
            MilliSleep(5000);
            
            // Check for shutdown signal
            if (fShutdown) {
                ShutdownRosettaServer();
                break;
            }
        }
    }
    catch (std::exception& e) {
        PrintException(&e, "ThreadRosettaServer()");
    }
    catch (...) {
        PrintException(NULL, "ThreadRosettaServer()");
    }
    LogPrintf("ThreadRosettaServer exited\n");
}

} // namespace Rosetta
