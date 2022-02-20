﻿#include <tclap/CmdLine.h>

#include "Peer.h"

int main(int argc, char *argv[]) {
	try {
		TCLAP::CmdLine cmd("Create a P2P Client", ' ');

		TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", true, 0, "int", cmd);
		TCLAP::ValueArg<std::string> sipArg("s", "serverIP", "The indexing server IP address this client should connect to", false, "localhost", "ip address", cmd);
		TCLAP::ValueArg<uint16_t> spArg("e", "serverPort", "The indexing server Port this client should use", false, 55555, "int", cmd);
		TCLAP::ValueArg<uint16_t> cpArg("c", "clientPort", "The client Port other peers should connect to", false, 44000, "int", cmd);
		TCLAP::ValueArg<std::string> fldrArg("f", "downloadFolder", "The local folder files should be uploaded and downloaded to", true, "", "directory", cmd);

		cmd.parse(argc, argv);

		Peer c(idArg.getValue(), cpArg.getValue(), sipArg.getValue(), spArg.getValue(), fldrArg.getValue()); // Create a Peer object
		c.start(); // Start the Peer object
		c.console(); // Start the Peer's interactive prompt
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
