#pragma once

#include <stdio.h>
#include <string>
#include <iostream>

#include "Exchanger.h"
#include "indexRPC.h"

namespace Console {
	using std::string;

	void run(Index::Indexer &indexer, Exchanger::Exchanger &exchanger);
}
