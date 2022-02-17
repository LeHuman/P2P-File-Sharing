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

	void Console::setPrompt(string prompt) {
		this->prompt = prompt;
	}

	void Console::addParser(parserFunc &parser) {
		parsers.push_back(parser);
	}

	static vector<string> tokenize(string line) {
		size_t pos = 0;
		vector<string> tokens;
		while ((pos = line.find(' ')) != string::npos) {
			tokens.push_back(line.substr(0, pos));
			line.erase(0, pos + 1);
		}
		tokens.push_back(line); // emplace instead?

		return tokens;
	}

	static bool interpret(string &line, std::vector<parserFunc *> &parsers, Index::Indexer *indexer, Exchanger::Exchanger *exchanger) {
		vector<string> tokens = tokenize(line);
		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
		string func = tokens[0];

		for (parserFunc *parser : parsers) {
			if (parser(indexer, exchanger, func, tokens)) {
				return true;
			}
		}

		if (func == "q" || func == "quit" || func == "exit") {
			Log.i(ID, "Exiting");
			return false;
		}

		Log.e(ID, "Action does not exist: %s", func.data());
		return true;
	}

	void Console::run(Index::Indexer *indexer, Exchanger::Exchanger *exchanger) {
		Log(ID, "Console start!");
		while (true) {
			std::cout << " " << prompt << " > ";
			std::getline(std::cin, line);
			try {
				if (!interpret(line, parsers, indexer, exchanger))
					break;
			} catch (const std::exception &e) {
				Log.e(ID, e.what());
			}
		}
	}
}
