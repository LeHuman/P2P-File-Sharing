#pragma once

#include <string>
#include <vector>

#include "indexRPC.h"

bool indexRPCFunc(RPC::Indexer &indexer, std::string func, std::vector<std::string> tokens);
