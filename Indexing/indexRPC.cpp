/**
 * @file indexRPC.cpp
 * @author IR
 * @brief Source code for the indexRPC module
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <thread>
#include <chrono>

#include "rpc/server.h"

#include "indexRPC.h"
#include <rpc/rpc_error.h>
#include <rpc/this_session.h>

namespace Index {
	Indexer::~Indexer() {
		if (srv != nullptr) {
			srv->stop();
			delete srv;
		}
		if (clt != nullptr)
			delete clt;
	}

	Indexer::Indexer(uint16_t port) {
		srv = new rpc::server(port);
		database = new Database();
		serverConn.ip = "127.0.0.1"; // DNC
		serverConn.port = port;
		isServer = true;
		bindFunctions();
	}

	Indexer::Indexer(int id, uint16_t cPort, string sIP, uint16_t sPort) {
		this->id = id;
		peerConn.ip = "127.0.0.1"; // Modified when connected to an indexing server
		peerConn.port = cPort;
		serverConn.ip = sIP;
		serverConn.port = sPort;
		isServer = false;
	}

	void Indexer::bindFunctions() {
		if (isServer) {
			srv->bind(k_Register, [&](int id, conn_t connection, string entryName, entryHash_t hash) -> bool {
				connection.ip = rpc::this_session().remoteAddr;
				return database->registry(id, connection, entryName, hash);
					  });
			srv->bind(k_Deregister, [&](int id, conn_t connection, entryHash_t hash) -> bool {
				connection.ip = rpc::this_session().remoteAddr;
				return database->deregister(id, connection, hash); });
			srv->bind(k_Search, [&](string query) -> EntryResults {return database->search(query); });
			srv->bind(k_List, [&]() -> EntryResults {return database->list(); });
			srv->bind(k_Request, [&](entryHash_t hash) -> PeerResults {return database->request(hash); });
			srv->bind(k_Ping, [&]() {return true;});
		}
	}

	void Indexer::stop() {
		if (isServer) {
			Log.i("Indexer", "Stopping Server");
			srv->stop();
			delete srv;
			srv = nullptr;
		} else {
			Log.i("Indexer", "Stopping Client");
			delete clt;
			clt = nullptr;
		}
	}

	void Indexer::start() {
		if (isServer) {
			Log.i("Indexer", "Running Server");
			srv->async_run(std::thread::hardware_concurrency());
		} else {
			Log.i("Indexer", "Running Client");
			while (true) {
				try {
					Log.d("Indexer", "Connecting to index server at: %s:%d", serverConn.ip, serverConn.port);
					clt = new rpc::client(serverConn.ip, serverConn.port);
					clt->set_timeout(6000);
					clt->call(k_Ping);
					Log.i("Indexer", "Server pinged! %fms", ping() / 1000.0);
					break;
				} catch (const rpc::system_error &e) {
					Log.f("Indexer", "Unable to create client: %s", e.what());
					delete clt;
					Log.i("Indexer", "Attempting to connect again in 5 seconds");
					std::this_thread::sleep_for(std::chrono::seconds(5));
				}
			}
		}
	}

	std::chrono::microseconds Indexer::ping() {
		if (!isServer) {
			auto start = std::chrono::high_resolution_clock::now();
			auto resp = clt->call(k_Ping);
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = duration_cast<std::chrono::microseconds>(stop - start);
			return duration;
		}
		return std::chrono::microseconds(-1);
	}

	bool Indexer::connected() {
		if (isServer)
			return true;
		return clt->get_connection_state() == rpc::client::connection_state::connected;
	}

	bool Indexer::registry(string entryName, entryHash_t hash) {
		if (isServer)
			return false;
		return clt->call(k_Register, id, peerConn, entryName, hash).as<bool>();
	}

	bool Indexer::deregister(entryHash_t hash) {
		if (isServer)
			return false;
		return clt->call(k_Deregister, id, peerConn, hash).as<bool>();
	}

	EntryResults Indexer::search(string query) {
		if (isServer)
			return database->search(query);
		return clt->call(k_Search, query).as<EntryResults>();
	}

	EntryResults Indexer::list() {
		if (isServer)
			return database->list();
		return clt->call(k_List).as<EntryResults>();
	}

	PeerResults Indexer::request(entryHash_t hash) {
		if (isServer)
			return database->request(hash);
		return clt->call(k_Request, hash).as<PeerResults>();
	}

	Database *Indexer::getDatabase() {
		return database;
	}
}
