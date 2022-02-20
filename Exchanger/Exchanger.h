/**
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

	/**
	 * @brief struct query_t is used to manager requests that are going to download a file
	*/
	struct query_t {
		Index::PeerResults results;
		string downloadPath;
		entryHash_t hash;
		query_t(Index::PeerResults results, string hash, string downloadPath) : results { results }, hash { hash }, downloadPath { downloadPath }{};
	};

	/**
	 * @brief Exchanger is used to transfer files between peers
	*/
	class Exchanger {
		asio::io_context *io_context = nullptr;

		bool running = true;

		std::mutex mutex;
		string downloadPath;
		std::condition_variable cond;
		std::queue<struct query_t> queries;
		std::unordered_map<Index::entryHash_t, Util::File> localFiles;

		void fileSender(uint32_t id, asio::ip::tcp::iostream stream);
		bool fileReceiver(asio::ip::tcp::iostream &stream, uint32_t eid, entryHash_t hash, string downloadPath);
		void listener(uint32_t id, uint16_t port);
		void peerResolver(Index::PeerResults peers, Index::entryHash_t hash, string downloadPath);
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
		Exchanger(uint32_t id, uint16_t listeningPort, string downloadPath);

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
		 * @param hash The hash of the file to download
		 * @Return download queued
		*/
		bool download(Index::PeerResults results, entryHash_t hash);

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
	};
}
