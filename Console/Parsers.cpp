/**
 * @file Parsers.cpp
 * @author IR
 * @brief The Parser source code
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Parsers.h"
#include "Log.h"

bool indexRPCFunc(Index::Indexer *indexer, Exchanger::Exchanger *exchanger, std::string func, std::vector<std::string> tokens) {
	static const std::string ID = "indexRPCParse";

    // Check if tokens match then run function
	if (func == Index::k_Deregister || func == Index::k_Register) { // Shouldn't be called directly
		Log.e(ID, "Not allowed to be called directly");
	} else if (func == Index::k_List) {
		Log.i(ID, "Listing server file index");
		for (Index::Entry::searchEntry &item : indexer->list()) {
			std::cout << item.str().data() << std::endl;
		}
	} else if (func == Index::k_Search) {
		Log.i(ID, "Searching server file index for: %s", tokens[1].data());
		for (Index::Entry::searchEntry &item : indexer->search(tokens[1])) {
			std::cout << item.str().data() << std::endl;
		}
	} else if (func == Index::k_Ping) {
		Log.i(ID, "Server reponse time: %fms", indexer->ping() / 1000.0);
	} else if (func == Index::k_Request) {
		Index::entryHash_t hash = tokens[1];
		Log.i(ID, "Searching server peer index for: %s", hash.data());
		Index::PeerResults results = indexer->request(hash);
		for (Index::Peer::searchEntry &item : results.peers) {
			std::cout << item.str().data() << std::endl;
		}
		exchanger->download(results, hash);
	} else {
		return false;
	}
	return true;
}
