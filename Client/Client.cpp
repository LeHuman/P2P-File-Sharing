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
#include <nlohmann/json.hpp>

#include "IndexRPC.h"
#include "Peer.h"
#include "Config.h"

int main(int argc, char *argv[]) {
	try {
		// Get command line arguments using library
		TCLAP::CmdLine cmd("Create a P2P Client using static config", ' ');

		TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", false, 0, "int", cmd);
		TCLAP::ValueArg<std::string> confArg("c", "configFile", "The config file to use", false, "../../../../test_config.json", "filePath", cmd);
		TCLAP::ValueArg<std::string> fldrArg("f", "downloadFolder", "The local folder files should be uploaded and downloaded to", false, "../../testFolder0", "directory", cmd);

		cmd.parse(argc, argv);

		Config::config_t config = Config::getConfig(idArg.getValue(), confArg.getValue());

		Index::Indexer *s;
		// Create a Peer object
		Peer c(idArg.getValue(), config.port + 1000, config.server.ip, config.server.port, fldrArg.getValue()); //1000 is added to port to differentiate client to server when on super peers

		if (config.isSuper) { // Create server if this is a super peer
			s = new Index::Indexer(config.server.port, config.totalSupers);
			for (Index::conn_t conn : config.neighbors)
				s->addNeighboor(conn);
			s->start();
		}

		c.start(); // Start the Peer object
		c.console(); // Start the Peer's interactive prompt
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
