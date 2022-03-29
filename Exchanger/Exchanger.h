﻿/**
 * @file Exchanger.h
 * @author IR
 * @brief Exchanger module deals with handeling the communication between peers
 * @version 0.1
 * @date 2022-02-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <stdint.h>
#include <condition_variable>
#include <string>
#include <queue>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "index.h"
#include "Folder.h"

namespace Exchanger {
	using std::string;
	using Index::entryHash_t;

	enum Mode : char {
		P2PFile,
		P2PFileUpdate,
		InvalidateFile
	};

	/**
	 * @brief struct query_t is used to manager requests that are going to download a file
	*/
	struct query_t {
		Index::PeerResults results;
		string downloadPath;
		std::string key;
		bool usingHash;
		query_t(Index::PeerResults results, string key, string downloadPath, bool usingHash) : results { results }, key { key }, downloadPath { downloadPath }, usingHash { usingHash }{};
	};

	/**
	 * @brief Directly push invalidation of a hash to a peer
	 */
	void fileInvalidate(const Index::Peer &peer, entryHash_t hash);

	/**
	 * @brief Exchanger is used to transfer files between peers
	*/
	class Exchanger {
		asio::io_context *io_context = nullptr;

		bool running = true;
		time_t _TTR = 0;

		std::mutex mutex;
		std::mutex localMutex;
		string downloadPath;
		std::condition_variable cond;
		std::queue<struct query_t> queries;
		std::unordered_map<Index::entryHash_t, Util::File> localFiles;
		std::unordered_map<std::string, Util::File> localFileNames;

		std::function<void(Util::File)> invalidationListener = nullptr;
		std::function<void(Util::File, Index::origin_t)> downloadListener = nullptr;
		std::function<void(entryHash_t, time_t)> TTRListener = nullptr;
		std::function<Index::origin_t(entryHash_t)> originHandler = nullptr;

		void fileSender(uint32_t id, asio::ip::tcp::iostream stream);
		int fileReceiver(asio::ip::tcp::iostream &stream, uint32_t eid, Index::entryHash_t hash, string downloadPath, bool usingHash);
		void listener(uint32_t id, uint16_t port);
		void peerResolver(Index::PeerResults peers, std::string key, string downloadPath, bool usingHash);
		void receiver();
		void _startSocket(uint32_t id, uint16_t listeningPort);

	public:

		~Exchanger();

		/**
		 * @brief Create a new Exchanger object
		 * @param id The unique ID of the peer making this object
		 * @param listeningPort The port that this peer should listen to
		 * @param downloadPath the directory to download/upload to/from
		*/
		Exchanger(uint32_t id, uint16_t listeningPort, string downloadPath, const std::function<void(Util::File, Index::origin_t)> &downloadListener, const std::function<void(Util::File)> &invalidationListener, std::function<Index::origin_t(entryHash_t)> originHandler, std::function<void(entryHash_t, time_t)> TTRListener);

		/**
		 * @brief stop this exchanger
		*/
		void stop();

		/**
		 * @brief change the directory to download/upload to/from
		 * @param downloadPath the directory to download/upload to/from
		*/

		void setDownloadPath(string downloadPath);

		/**
		 * @brief download a file from a peer
		 * @param results The results from an index server
		 * @param key The hash of the file to download or alternatively the filename
		 * @param usingHash whether to search using a hash, else, use a filename
		 * @Return download queued
		*/
		bool download(Index::PeerResults results, std::string hash, bool usingHash = true);

		/**
		 * @brief Adds a file to the exchangers known list of files. Used to actually pass to another peer
		 *
		 * @param file The file to store
		 */
		void addLocalFile(Util::File file);

		/**
		 * @brief Removes a file from the exchangers known list of files.
		 *
		 * @param file The file to remove
		 */
		void removeLocalFile(Util::File file);

		/**
		 * @brief Updates a file if it was modified
		 *
		 * @param file The file to update
		 */
		void updateLocalFile(Util::File file);

		void setDefaultTTR(time_t TTR);
	};
}
