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
#include "index.h"
#include "Exchanger.h"

 /**
  * @brief Class representing a peer
 */
class Peer {
	uint32_t id;
	uint16_t listeningPort;
	std::string indexingIP;
	uint16_t indexingPort;
	std::string uploadPath, downloadPath;
	bool pulling = false; // Enable pull mode for consistency checking
	bool pushing = false; // Enable push mode for consistency checking
	time_t _TTR = 0;

	Util::Folder *originWatcher = nullptr;
	Util::Folder *remoteWatcher = nullptr;
	Index::Indexer *indexer = nullptr;
	Exchanger::Exchanger *exchanger = nullptr;
	Console::Console _console;
/*--------- start change ----------*/
	void registerFile(std::string fileName, Index::entryHash_t hash, Index::origin_t origin);

	void deregisterFile(Index::entryHash_t hash, bool master = false);

	void invalidateFile(Index::entryHash_t hash);

	void originFolderListener(Util::File file, Util::File::Status status);

	void remoteFolderListener(Util::File file, Util::File::Status status);

	void invalidationListener(Util::File file);

	void pullingListener(Index::Entry::searchEntry entry);

	void downloadListener(Util::File file, Index::origin_t origin);

	Index::origin_t originHandler(Index::entryHash_t hash);
/*--------- end change ----------*/
public:
	~Peer();

	/**
	 * @brief Create a new peer
	 * @param id The unique id of this peer
	 * @param listeningPort The external port other peers should connect to
	 * @param indexingIP The IP address of the indexing server
	 * @param indexingPort The port of the indexing server
	 * @param uploadPath The path to the folder to upload from.
	 * @param downloadPath The path to the folder to download to.
	*/
	Peer(uint32_t id, uint16_t listeningPort, std::string indexingIP, uint16_t indexingPort, std::string uploadPath, std::string downloadPath, bool pushing, bool pulling, time_t TTR);

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
