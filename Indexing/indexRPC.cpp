#include <thread>

#include "indexRPC.h"

namespace Index {
	Indexer::~Indexer() {
		if (srv != nullptr)
			delete srv;
		if (clt != nullptr)
			delete clt;
	}

	Indexer::Indexer(uint16_t port) {
		srv = new rpc::server(port);
		database = new Database();
		conn.port = port;
		isServer = true;
		bindFunctions();
	}

	Indexer::Indexer(int id, string ip, uint16_t port) {
		this->id = id;
		conn.ip = ip;
		conn.port = port;
		isServer = false;
	}

	void Indexer::bindFunctions() {
		if (isServer) {
			srv->bind(k_Register, [&](int id, conn_t connection, string entryName, entryHash_t hash) -> bool { return database->registry(id, connection, entryName, hash); });
			srv->bind(k_Deregister, [&](int id, conn_t connection, entryHash_t hash) -> bool {return database->deregister(id, connection, hash); });
			srv->bind(k_Search, [&](string query) -> EntryResults {return database->search(query); });
			srv->bind(k_List, [&]() -> EntryResults {return database->list(); });
			srv->bind(k_Request, [&](entryHash_t hash) -> PeerResults {return database->request(hash); });
			srv->bind(k_Ping, [&]() {return true; });
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
			clt = new rpc::client(conn.ip, conn.port);
			Log.i("Indexer", "Server pinged! %dms", ping());
		}
	}

	milliseconds Indexer::ping() {
		if (!isServer) {
			auto start = std::chrono::high_resolution_clock::now();
			auto resp = clt->call(k_Ping);
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(stop - start);
			return duration;
		}
		return milliseconds(-1);
	}

	bool Indexer::registry(string entryName, entryHash_t hash) {
		if (isServer)
			return false;
		return clt->call(k_Register, id, conn, entryName, hash).as<bool>();
	}

	bool Indexer::deregister(entryHash_t hash) {
		if (isServer)
			return false;
		return clt->call(k_Deregister, id, conn, hash).as<bool>();
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
