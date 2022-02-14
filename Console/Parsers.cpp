#include "Parsers.h"
#include "Log.h"

bool indexRPCFunc(Index::Indexer &indexer, Exchanger::Exchanger &exchanger, std::string func, std::vector<std::string> tokens) {
	static const std::string ID = "indexRPCParse";

	if (func == Index::k_Deregister || func == Index::k_Register) { // Shouldn't be called directly
		Log.e(ID, "Not allowed to be called directly");
	} else if (func == Index::k_List) {
		Log.i(ID, "Listing server file index");
		for (Index::Entry::searchEntry &item : indexer.list()) {
			std::cout << item.str().data() << std::endl;
		}
	} else if (func == Index::k_Search) {
		Log.i(ID, "Searching server file index for: %s", tokens[1].data());
		for (Index::Entry::searchEntry &item : indexer.search(tokens[1])) {
			std::cout << item.str().data() << std::endl;
		}
	} else if (func == Index::k_Ping) {
		Log.i(ID, "Server reponse time: %dms", indexer.ping());
	} else if (func == Index::k_Request) { // Should call async to start downloading from sources / a source
		Log.i(ID, "Searching server peer index for: %s", tokens[1].data());
		for (Index::Peer::searchEntry &item : indexer.request((Index::entryHash_t)tokens[1])) {
			std::cout << item.str().data() << std::endl;
		}
	} else {
		return false;
	}
	return true;
}
