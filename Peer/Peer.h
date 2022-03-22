/**
 * @file Peer.h
 * @author IR
 * @brief Class that deals with everything a peer needs
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <string>

#include "Log.h"
#include "indexRPC.h"
#include "Console.h"
#include "Exchanger.h"

/**
 * @brief Class representing a peer
*/
class Peer {
	uint32_t id;
	uint16_t listeningPort;
	std::string indexingIP;
	uint16_t indexingPort;
	std::string downloadPath;

	Util::Folder *folderWatcher = nullptr;
	Index::Indexer *indexer = nullptr;
	Exchanger::Exchanger *exchanger = nullptr;
	Console::Console _console;

	void registerFile(Index::Indexer &indexer, std::string fileName, Index::entryHash_t hash);

	void deregisterFile(Index::Indexer &indexer, Index::entryHash_t hash);

	void invalidateFile(Index::Indexer &indexer, Index::entryHash_t hash);

	void originFolderListener(Util::File file, Util::File::Status status);

public:
	~Peer();

	/**
	 * @brief Create a new peer
	 * @param id The unique id of this peer
	 * @param listeningPort The external port other peers should connect to
	 * @param indexingIP The IP address of the indexing server
	 * @param indexingPort The port of the indexing server
	 * @param downloadPath The path to the folder to download and update to.
	*/
	Peer(uint32_t id, uint16_t listeningPort, std::string indexingIP, uint16_t indexingPort, std::string downloadPath);

	/**
	 * @brief Switch to this Peer's interactive console
	*/
	void console();

	/**
	 * @brief Start this peer, initializing connections and folder watching
	*/
	void start();

	/**
	 * @brief Stop this peer and all related connections
	*/
	void stop();
};
