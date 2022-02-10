#pragma once

#include <vector>
#include <string>

#include "rpc/server.h"
#include "rpc/client.h"

#include "Log.h"
#include "index.h"

namespace RPC {
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
		Index::conn_t conn;
		bool isServer;
		int id = -1;

		/**
		 * @brief Binds the server functions to be used by clients
		*/
		void bindFunctions();

	public:
		~Indexer();

		/**
		 * @brief Create an Indexer Server
		 * @param port port used to listen on
		*/
		Indexer(uint16_t port);

		/**
		 * @brief Create an Indexer Client
		 * @param id unique id identifying this client
		 * @param ip ip to connect to
		 * @param port port to connect to
		*/
		Indexer(int id, string ip, uint16_t port);

		/**
		 * @brief Start the server/client connection
		 * @note Is non-blocking
		*/
		void start();

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
	};
}
