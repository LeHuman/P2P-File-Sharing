#pragma once

#include <stdio.h>
#include <string>
#include <iostream>

#include "Exchanger.h"
#include "indexRPC.h"

namespace Console {
	using std::string;

	typedef bool parserFunc(Index::Indexer *indexer, Exchanger::Exchanger *exchanger, std::string func, std::vector<std::string> tokens);

	/**
	 * @brief Console class creates an interactive prompt that allows to interact with a peer in
	 * relation to it's connection to the index server and requesting files to download from other peers
	*/
	class Console {
		string prompt = "Console";
		string line;

		std::vector<parserFunc *> parsers;
	public:
		/**
		 * @brief Set the prompt that shows up before each command
		 * @param prompt The string to show
		 * @note Prompt is a bit finicky due to concurrent operations on the same terminal
		*/
		void setPrompt(string prompt);

		/**
		 * @brief Add a parser for the interactive prompt
		 * @param parser The parser function that can take the interactive prompt tokens
		*/
		void addParser(parserFunc &parser);

		/**
		 * @brief Activate the interactive prompt for this console
		 * @param indexer The peer indexer
		 * @param exchanger The peer exchanger
		*/
		void run(Index::Indexer *indexer, Exchanger::Exchanger *exchanger);
	};
}
