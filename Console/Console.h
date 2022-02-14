#pragma once

#include <stdio.h>
#include <string>
#include <iostream>

#include "indexRPC.h"
#include "Exchanger.h"

namespace Console {
	using std::string;

	void run(Index::Indexer &indexer, Exchanger::Exchanger &exchanger);
}
