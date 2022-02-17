#pragma once

#include <string>

#include "Log.h"
#include "indexRPC.h"
#include "Console.h"
#include "Exchanger.h"

class Peer {
	uint32_t id;
	std::string externalIP;
	uint16_t listeningPort;
	std::string indexingIP;
	uint16_t indexingPort;
	std::string downloadPath;

	std::thread *folderWatcher = nullptr;
	Index::Indexer *indexer = nullptr;
	Exchanger::Exchanger *exchanger = nullptr;
	Console::Console _console;

	void registerFile(Index::Indexer &indexer, std::string fileName, Index::entryHash_t hash);

	void deregisterFile(Index::Indexer &indexer, Index::entryHash_t hash);

	void listener(Util::File file, Util::File::Status status);

public:

	~Peer();

	Peer(uint32_t id, std::string externalIP, uint16_t listeningPort, std::string indexingIP, uint16_t indexingPort, std::string downloadPath);

	void console();

	void start();

	void stop();
};
