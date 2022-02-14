#include <stdio.h>
#include <vector>
#include <algorithm>

#include "Console.h"
#include "index.h"
#include "Log.h"
#include "Parsers.h"

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

	void interpret(Index::Indexer &indexer) {
		vector<string> tokens = tokenize();
		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
		string func = tokens[0];

		// Add other parsers here
		if (indexRPCFunc(indexer, func, tokens))
			return;

		Log.e(ID, "Action does not exist: %s", func.data());
	}

	void run(Index::Indexer &indexer) {
		Log(ID, "Console start!");
		while (true) {
			std::getline(std::cin, line);
			interpret(indexer);
		}
	}
}
