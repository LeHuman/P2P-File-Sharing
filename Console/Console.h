﻿#pragma once

#include <stdio.h>
#include <string>
#include <iostream>

#include "Exchanger.h"
#include "indexRPC.h"

namespace Console {
	using std::string;

	typedef bool parserFunc(Index::Indexer& indexer, Exchanger::Exchanger& exchanger, std::string func, std::vector<std::string> tokens);

	void setPrompt(string prompt);

	void addParser(parserFunc& parser);

	void run(Index::Indexer &indexer, Exchanger::Exchanger &exchanger);
}
