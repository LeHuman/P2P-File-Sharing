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

#include <string>
#include <filesystem>

#include <tclap/CmdLine.h>
#include <nlohmann/json.hpp>

#include "IndexRPC.h"
#include "Peer.h"
#include "Config.h"

using std::string;

int main(int argc, char *argv[]) {
	try {
		// Get command line arguments using library
		TCLAP::CmdLine cmd("Create a P2P Client using static config", ' ');

		TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", false, 0, "int", cmd);
		TCLAP::ValueArg<std::string> confArg("c", "configFile", "The config file to use", false, "test_config.json", "filePath", cmd);
		TCLAP::ValueArg<std::string> upFldrArg("u", "uploadFolder", "The local folder files should be uploaded from", false, "", "directory", cmd);
		TCLAP::ValueArg<std::string> dnFldrArg("d", "downloadFolder", "The local folder files should be downloaded to", false, "", "directory", cmd);
		TCLAP::SwitchArg all2all("a", "all2all", "enable all2all mode", cmd);

		cmd.parse(argc, argv);

		Config::config_t config;

		try {
			config = Config::getConfig(idArg.getValue(), confArg.getValue(), all2all.getValue());
		} catch (const std::exception &e) {
			Log.e("Client", "Error reading config file: %s", e.what());
			exit(1);
		}

		string originFolder = upFldrArg.getValue();

		if (originFolder == "") {
			originFolder = "originFolder" + std::to_string(idArg.getValue());
		}

		if (!std::filesystem::exists(originFolder)) {
			std::filesystem::create_directories(originFolder);
			Log.w("Client", "Watch Folder auto created");
		}

		string remoteFolder = dnFldrArg.getValue();

		if (remoteFolder == "") {
			remoteFolder = "remoteFolder" + std::to_string(idArg.getValue());
		}

		if (!std::filesystem::exists(remoteFolder)) {
			std::filesystem::create_directories(remoteFolder);
			Log.w("Client", "Watch Folder auto created");
		}

		// Alloc ptr for super peer
		Index::Indexer *s;

		// Create a Peer object
		Peer c(idArg.getValue(), config.port + 1000, config.server.ip, config.server.port, originFolder, remoteFolder, config.pushing, config.pulling); //1000 is added to port to differentiate client to server when on super peers

		if (config.isSuper) { // Create server if this is a super peer
			s = new Index::Indexer(config.server.port, config.totalSupers, config.pushing, config.pulling, config.all2all);
			for (Index::conn_t conn : config.neighbors) {
				if (conn != config.server) { // Ensure we don't reference ourselves
					s->addNeighboor(conn);
				}
			}
			s->start();
		}

		c.start(); // Start the Peer object
		c.console(); // Start the Peer's interactive prompt
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
