/**
 * @file indexRPC.h
 * @author IR
 * @brief Module that deals with communication between peer and server using RPC
 * @version 0.1
 * @date 2022-02-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <unordered_set>

#include "rpc/server.h"
#include "rpc/client.h"

#include "Log.h"
#include "index.h"

namespace Index {
	using std::vector;
	using std::string;
	using std::unordered_set;
	using std::chrono::milliseconds;
	using Index::conn_t;
	using Index::EntryResults;
	using Index::PeerResults;
	using Index::entryHash_t;
	using logic_t = uint64_t;
	using uid_t = uint64_t;

	const string k_Register = "register";
	const string k_Deregister = "deregister";
	const string k_Search = "search";
	const string k_List = "list";
	const string k_Request = "request";
	const string k_Ping = "ping";

	/**
	 * @brief Indexer class used for communicating with server/client indexing using RPC
	*/
	class Indexer {
		rpc::client *clt = nullptr;
		rpc::server *srv = nullptr;
		Index::Database *database = nullptr;
		unordered_set<uid_t> UIDs;
		vector<conn_t> neighbors; // TODO: populate with config file
		conn_t serverConn;
		conn_t peerConn;
		bool isServer = false;
		int32_t _TTL;
		int id = -1;
		std::mutex uidMux;
		logic_t LC = 0; // Logic clock

		/**
		 * @brief Binds the server functions to be used by clients
		*/
		void bindFunctions();

		/**
		 * @brief Hashes both id and LC to return the next UID. Used internally for query propagation
		*/
		uid_t nextUID();

	public:
		~Indexer();

		Indexer() {};

		/**
		 * @brief Create an Indexer Server
		 *
		 * @param sPort port this indexing server should listen on
		 * @param totalSupers Number of total super peers on the static network, including this one
		 */
		Indexer(uint16_t sPort, int32_t totalSupers);

		/**
		 * @brief Create an Indexer Client
		 * @param id	Unique ID identifying this client
		 * @param cPort The client Port other peers should connect to
		 * @param sIP	The indexing server IP address this client should connect to
		 * @param sPort The indexing server Port this client should use
		*/
		Indexer(int id, uint16_t cPort, string sIP, uint16_t sPort);

		/**
		 * @brief Add a neighbor's connection info
		*/
		void addNeighboor(conn_t neighbor);

		/**
		 * @brief Start the server/client connection
		 * @note Is non-blocking
		*/
		void start();

		/**
		 * @brief Stop the server/client connection
		*/
		void stop();

		/**
		 * @brief Client can ping server for delay
		 * @return delay to get response in microseconds
		*/
		std::chrono::microseconds ping();

		/**
		 * @brief Check if client is connected
		 * @return true if connected
		*/
		bool connected();

		/**
		 * @brief Register file to index server
		 * @param entryName Name of the file
		 * @param hash hash of the file
		 * @return Sucessfully registered
		*/
		bool registry(string entryName, entryHash_t hash);

		/**
		 * @brief Deregister a file from an index server
		 * @param hash hash of the file
		 * @return Sucessfully deregistered
		*/
		bool deregister(entryHash_t hash);

		/**
		 * @brief Search index server for files that contain query
		 * @param query search query
		 * @return Vector of entries and their hashes
		*/
		EntryResults search(string query);

		/**
		 * @brief Get a list of every listed file on the index server
		 * @warning This can take a while if index has alot of files
		 *
		 * @return Vector of entries and their hashes
		*/
		EntryResults list();

		/**
		 * @brief Get a list of every peer that has a specific file
		 * @param hash hash of the file to get
		 * @return Vector of peers and their connection info
		*/
		PeerResults request(entryHash_t hash);

		/**
		 * @brief Return the current database, if there is one
		 * @return database or nullptr if this is a client
		*/
		Database *getDatabase();
	};
}
