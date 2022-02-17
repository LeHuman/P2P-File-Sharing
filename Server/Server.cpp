﻿#include <tclap/CmdLine.h>
#include "indexRPC.h"

int main(int argc, char *argv[]) {
	try {
		TCLAP::CmdLine cmd("Create a P2P Server", ' ');
		TCLAP::ValueArg<uint16_t> portArg("p", "port", "The port this server should listen to", true, 55555, "int", cmd);
		cmd.parse(argc, argv);

		Index::Indexer s(portArg.getValue());
		s.start();

		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
