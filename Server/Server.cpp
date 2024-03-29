﻿/**
 * @file Server.cpp
 * @author IR
 * @brief The server executable
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <tclap/CmdLine.h>
#include "indexRPC.h"

int main(int argc, char *argv[]) {
	try {
        // Get command line arguments using library
		TCLAP::CmdLine cmd("Create a P2P Server", ' ');
		TCLAP::ValueArg<uint16_t> portArg("p", "port", "The port this server should listen to", false, 55555, "int", cmd);
		cmd.parse(argc, argv);

		Index::Indexer s(portArg.getValue()); // Create Indexing server given port from the arguments or from default
		s.start(); // Start the Indexing server

		while (true) { // Wait indefinitely
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
