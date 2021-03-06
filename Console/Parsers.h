/**
 * @file Parsers.h
 * @author IR
 * @brief Parsers used for the console module
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <string>
#include <vector>

#include "indexRPC.h"
#include "Exchanger.h"

/**
 * @brief Used to parse RPC function calls to the indexing server
 * @param indexer The peer indexer
 * @param exchanger The peer exchanger
 * @param func The string of function being called, aka the first token
 * @param tokens The arguments passed to this function
 * @return Whether this function sucessfully parsed the given tokens
*/
bool indexRPCFunc(Index::Indexer *indexer, Exchanger::Exchanger *exchanger, std::string func, std::vector<std::string> tokens);
