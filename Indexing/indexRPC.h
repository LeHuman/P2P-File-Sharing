#pragma once

#include <vector>
#include <string>

#include "rpc/server.h"
#include "rpc/client.h"

#include "Log.h"
#include "index.h"

namespace Index {
	using std::vector;
	using std::string;
	using std::chrono::milliseconds;
	using Index::conn_t;
	using Index::EntryResults;
	using Index::PeerResults;
	using Index::entryHash_t;

	const string k_Register = "register";
	const string k_Deregister = "deregister";
	const string k_Search = "search";
	const string k_List = "list";
	const string k_Request = "request";
	const string k_Ping = "ping";

	class Indexer {
		rpc::client *clt = nullptr;
		rpc::server *srv = nullptr;
		Index::Database *database = nullptr;
		Index::conn_t serverConn;
		Index::conn_t peerConn;
		bool isServer = false;
		int id = -1;

		/**
		 * @brief Binds the server functions to be used by clients
		*/
		void bindFunctions();

	public:
		~Indexer();

		Indexer() {};

		/**
		 * @brief Create an Indexer Server
		 * @param port port this indexing server should listen on
		*/
		Indexer(uint16_t port);

		/**
		 * @brief Create an Indexer Client
		 * @param id	Unique ID identifying this client
		 * @param cIP	The client IP address other peers should connect to
		 * @param cPort The client Port other peers should connect to
		 * @param sIP	The indexing server IP address this client should connect to
		 * @param sPort The indexing server Port this client should use
		*/
		Indexer(int id, string cIP, uint16_t cPort, string sIP, uint16_t sPort);

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
		 * @return delay to get response in milliseconds
		*/
		milliseconds ping();

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
