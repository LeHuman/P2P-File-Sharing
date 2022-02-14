#pragma once

#include <string>
#include <vector>

#include "indexRPC.h"

bool indexRPCFunc(Index::Indexer &indexer, std::string func, std::vector<std::string> tokens);
