#include <tclap/CmdLine.h>

#include "Peer.h"

int main(int argc, char *argv[]) {
	try {
		TCLAP::CmdLine cmd("Create a P2P Client", ' ');

		TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", true, 0, "int", cmd);
		TCLAP::ValueArg<uint16_t> cpArg("c", "clientPort", "The client Port other peers should connect to", true, 55555, "int", cmd);
		TCLAP::ValueArg<uint16_t> spArg("s", "serverPort", "The indexing server Port this client should use", true, 55555, "int", cmd);
		TCLAP::ValueArg<std::string> cipArg("l", "clientIP", "The client IP address other peers should connect to", true, "localhost", "ip address", cmd);
		TCLAP::ValueArg<std::string> sipArg("e", "serverIP", "The indexing server IP address this client should connect to", true, "localhost", "ip address", cmd);
		TCLAP::ValueArg<std::string> fldrArg("f", "downloadFolder", "The local folder files should be uploaded and downlaoded to", true, "localhost", "ip address", cmd);

		cmd.parse(argc, argv);

		Peer c(idArg.getValue(), cipArg.getValue(), cpArg.getValue(), sipArg.getValue(), spArg.getValue(), fldrArg.getValue());
		c.start();
		c.console();
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return 0;
}
