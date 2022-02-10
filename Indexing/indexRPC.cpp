#include <thread>

#include "indexRPC.h"

namespace RPC {
	Indexer::~Indexer() {
		if (srv != nullptr)
			delete srv;
		if (clt != nullptr)
			delete clt;
	}

	Indexer::Indexer(uint16_t port) {
		srv = new rpc::server(port);
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

	bool _ping() {
		return true;
	}

	void Indexer::bindFunctions() {
		if (isServer) {
			srv->bind(k_Register, Index::registry);
			srv->bind(k_Deregister, Index::deregister);
			srv->bind(k_Search, Index::search);
			srv->bind(k_List, Index::list);
			srv->bind(k_Request, Index::request);
			srv->bind(k_Ping, _ping);
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
		auto state = clt->get_connection_state();
		return clt->call(k_Register, id, conn, entryName, hash).as<bool>();
	}

	bool Indexer::deregister(entryHash_t hash) {
		return clt->call(k_Deregister, id, conn, hash).as<bool>();
	}

	EntryResults Indexer::search(string query) {
		return clt->call(k_Search, query).as<EntryResults>();
	}

	EntryResults Indexer::list() {
		return clt->call(k_List).as<EntryResults>();
	}

	PeerResults Indexer::request(entryHash_t hash) {
		return clt->call(k_Request, hash).as<PeerResults>();
	}
}
