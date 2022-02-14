#pragma once

#include <string>
#include <vector>

#include "indexRPC.h"
#include "Exchanger.h"

bool indexRPCFunc(Index::Indexer &indexer, Exchanger::Exchanger &exchanger, std::string func, std::vector<std::string> tokens);
