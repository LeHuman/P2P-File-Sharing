#include <stdio.h>
#include <vector>
#include <algorithm>

#include "Console.h"
#include "index.h"
#include "Log.h"

namespace Console {
	using std::vector;

	static const string ID = "Console";

	string line;

	vector<string> tokenize() {
		size_t pos = 0;
		vector<string> tokens;
		while ((pos = line.find(' ')) != string::npos) {
			tokens.push_back(line.substr(0, pos));
			line.erase(0, pos + 1);
		}
		tokens.push_back(line); // emplace instead?

		return tokens;
	}

	void interpret(RPC::Indexer &indexer) {
		vector<string> tokens = tokenize();
		string func = tokens[0];
		transform(func.begin(), func.end(), func.begin(), ::tolower);

		if (func == RPC::k_Deregister) { // Shouldn't be called directly?
			bool deregistered = indexer.deregister((Index::entryHash_t)tokens[1]);
			if (deregistered) {
				Log.i(ID, "Deregisted hash: %s", tokens[1].data());
			} else {
				Log.e(ID, "Unable to deregister hash: %s", tokens[1].data());
			}
		} else if (func == RPC::k_Register) { // Shouldn't be called directly?
			bool registered = indexer.registry(tokens[1], (Index::entryHash_t)tokens[2]);
			if (registered) {
				Log.i(ID, "Registered hash: %s", tokens[1].data());
			} else {
				Log.e(ID, "Unable to registered hash: %s", tokens[1].data());
			}
		} else if (func == RPC::k_List) {
			Log.i(ID, "Listing server file index");
			for (Index::Entry::searchEntry &item : indexer.list()) {
				std::cout << item.str().data() << std::endl;
			}
		} else if (func == RPC::k_Search) {
			Log.i(ID, "Searching server file index for: %s", tokens[1].data());
			for (Index::Entry::searchEntry &item : indexer.search(tokens[1])) {
				std::cout << item.str().data() << std::endl;
			}
		} else if (func == RPC::k_Ping) {
			Log.i(ID, "Server reponse time: %dms", indexer.ping());
		} else if (func == RPC::k_Request) { // Should call async to start downloading
			Log.i(ID, "Searching server peer index for: %s", tokens[1].data());
			for (Index::Peer::searchEntry &item : indexer.request((Index::entryHash_t)tokens[1])) {
				std::cout << item.str().data() << std::endl;
			}
		} else {
			Log.e(ID, "Action does not exist: %s", func.data());
		}
	}

	void run(RPC::Indexer &indexer) {
		Log(ID, "Console start!");
		while (true) {
			std::getline(std::cin, line);
			interpret(indexer);
		}
	}
}
