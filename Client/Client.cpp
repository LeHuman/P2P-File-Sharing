/**
 * @file Client.cpp
 * @author IR
 * @brief The client executable
 * @version 0.2
 * @date 2022-02-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <tclap/CmdLine.h>

#include "Peer.h"

int main(int argc, char *argv[]) {
	try {
		// Get command line arguments using library
		TCLAP::CmdLine cmd("Create a P2P Client", ' ');

		TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", true, 0, "int", cmd);
		TCLAP::ValueArg<std::string> sipArg("s", "serverIP", "The indexing server IP address this client should connect to", false, "localhost", "ip address", cmd);
		TCLAP::ValueArg<uint16_t> spArg("e", "serverPort", "The indexing server Port this client should use", false, 55555, "int", cmd);
		TCLAP::ValueArg<uint16_t> cpArg("c", "clientPort", "The client Port other peers should connect to", false, 0, "int", cmd);
		TCLAP::ValueArg<std::string> fldrArg("f", "downloadFolder", "The local folder files should be uploaded and downloaded to", true, "", "directory", cmd);

		cmd.parse(argc, argv);

		uint16_t cPort = cpArg.getValue();
		if (cPort == 0)
			cPort = 44000 + idArg.getValue();

		Peer c(idArg.getValue(), cPort, sipArg.getValue(), spArg.getValue(), fldrArg.getValue()); // Create a Peer object
		c.start(); // Start the Peer object
		c.console(); // Start the Peer's interactive prompt
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
