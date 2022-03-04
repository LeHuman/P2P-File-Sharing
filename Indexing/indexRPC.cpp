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
#include "rpc/this_handler.h"

#include "indexRPC.h"
#include <rpc/rpc_error.h>
#include <rpc/this_session.h>

namespace Index {

	static const int64_t timeout = 10000;

	Indexer::~Indexer() {
		if (srv != nullptr) {
			srv->stop();
			delete srv;
		}
		if (clt != nullptr)
			delete clt;
	}

	Indexer::Indexer(uint16_t sPort, int32_t totalSupers, bool all2all) : all2all { all2all } {
		_TTL = totalSupers;
		srv = new rpc::server(sPort);
		database = new Database();
		serverConn.ip = "127.0.0.1"; // DNC
		serverConn.port = sPort;
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

	void searchEntries(uid_t &uid, int32_t &TTL, string &query, conn_t &neighbor, EntryResults &R, unsigned short _port) {
		try {
			Log.d("Indexer", "Propagating query %s %llu %u", k_Search.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			//Log.d("Indexer", "Calling %llu", uid);
			EntryResults _R = clt.call(k_Search, uid, TTL, query, _port).as<EntryResults>();
			EntryResults _Ra;

			for (Entry::searchEntry &rs : _R) { // TODO: O(n^2)
				bool found = false;
				for (Entry::searchEntry &ls : R) {
					if (ls == rs) {
						ls += rs;
						found = true;
						break; // There should at most be one exact duplicate per entry
					}
				}
				if (!found) {
					_Ra.push_back(rs);
				}
			}

			R.insert(R.end(), _Ra.begin(), _Ra.end());
			//R.insert(R.end(), _R.begin(), _R.end());
			//Log.d("Indexer", "Calling Finished %llu", uid);
		} catch (const std::runtime_error &e) {
			Log.w("Indexer", "Neighbor error: %s:%u %s", neighbor.ip.data(), neighbor.port, e.what());
		}
	}

	void listEntries(uid_t &uid, int32_t &TTL, conn_t &neighbor, EntryResults &R, unsigned short _port) {
		try {
			Log.d("Indexer", "Propagating query %s %llu %u", k_List.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			//Log.d("Indexer", "Calling %llu", uid);
			EntryResults _R = clt.call(k_List, uid, TTL, _port).as<EntryResults>();
			EntryResults _Ra;

			for (Entry::searchEntry &rs : _R) { // TODO: O(n^2)
				bool found = false;
				for (Entry::searchEntry &ls : R) {
					if (ls == rs) {
						ls += rs;
						found = true;
						break; // There should at most be one exact duplicate per entry
					}
				}
				if (!found) {
					_Ra.push_back(rs);
				}
			}

			R.insert(R.end(), _Ra.begin(), _Ra.end());
			//R.insert(R.end(), _R.begin(), _R.end());
			//Log.d("Indexer", "Calling Finished %llu", uid);
		} catch (const std::runtime_error &e) { // TODO: catch custom error
			Log.w("Indexer", "Neighbor error: %s", e.what());
		}
	}

	void requestPeers(uid_t &uid, int32_t &TTL, string &hash, conn_t &neighbor, PeerResults &R, unsigned short _port) {
		try {
			Log.d("Indexer", "Propagating query %s %llu %u", k_Request.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			//Log.d("Indexer", "Calling %llu", uid);
			auto _R = clt.call(k_Request, uid, TTL, hash, _port).as<PeerResults>();
			//Log.d("Indexer", "Calling Finished %llu", uid);
			R += _R;
		} catch (const std::runtime_error &e) { // TODO: catch custom error
			Log.w("Indexer", "Neighbor error: %s", e.what());
		}
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
			srv->bind(k_Ping, [&]() {return true; });

			// Propagated requests

			srv->bind(k_Search, [&](uid_t uid, int32_t TTL, string query, unsigned short _port) -> EntryResults {
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					// Returns
				}
				UIDs.insert(uid);
				uidMux.unlock();

				auto R = database->search(query);

				if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
					if (all2all) {
						TTL = 0; // propagated calls will not propagate any further
						for (conn_t neighbor : neighbors) {
							listEntries(uid, TTL, neighbor, R, serverConn.port);
						}
					} else {
						TTL = _TTL;
					}
				}

				TTL--;

				if (TTL > 0) {
					// Get connection info of this neighbor/peer to ensure we don't propagate backwards
					string _addr = rpc::this_session().remoteAddr;

					conn_t neighbor;

					for (conn_t n : neighbors) {
						if (n.ip != _addr || n.port != _port) { // Either differing attribute means we aren't propagating to the same super peer
							neighbor = n;
							break;
						}
					}

					searchEntries(uid, TTL, query, neighbor, R, serverConn.port);
				}

				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return R;
					  });

			srv->bind(k_List, [&](uid_t uid, int32_t TTL, unsigned short _port) -> EntryResults {
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					// Returns
				}
				UIDs.insert(uid);
				uidMux.unlock();

				auto R = database->list();

				if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
					if (all2all) {
						TTL = 0; // propagated calls will not propagate any further
						for (conn_t neighbor : neighbors) {
							listEntries(uid, TTL, neighbor, R, serverConn.port);
						}
					} else {
						TTL = _TTL;
					}
				}

				TTL--;

				if (TTL > 0) {
					// Get connection info of this neighbor/peer to ensure we don't propagate backwards
					string _addr = rpc::this_session().remoteAddr;

					conn_t neighbor;

					for (auto n : neighbors) {
						if (n.ip != _addr || n.port != _port) { // Either differing attribute means we aren't propagating to the same super peer
							neighbor = n;
							break;
						}
					}

					listEntries(uid, TTL, neighbor, R, serverConn.port);
				}

				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return R;
					  });

			srv->bind(k_Request, [&](uid_t uid, int32_t TTL, entryHash_t hash, unsigned short _port) -> PeerResults {
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					// Returns
				}
				UIDs.insert(uid);
				uidMux.unlock();

				auto R = database->request(hash);

				if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
					if (all2all) {
						TTL = 0; // propagated calls will not propagate any further
						for (conn_t neighbor : neighbors) {
							requestPeers(uid, TTL, hash, neighbor, R, serverConn.port);
						}
					} else {
						TTL = _TTL;
					}
				}

				TTL--;

				if (TTL > 0) {
					// Get connection info of this neighbor/peer to ensure we don't propagate backwards
					string _addr = rpc::this_session().remoteAddr;

					conn_t neighbor;

					for (auto n : neighbors) {
						if (n.ip != _addr || n.port != _port) { // Either differing attribute means we aren't propagating to the same super peer
							neighbor = n;
							break;
						}
					}

					requestPeers(uid, TTL, hash, neighbor, R, serverConn.port);
				}

				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return R;
					  });
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
			srv->async_run(std::thread::hardware_concurrency()*2);
		} else {
			Log.i("Indexer", "Running Client");
			while (true) {
				try {
					Log.d("Indexer", "Connecting to index server at: %s:%d", serverConn.ip.data(), serverConn.port);
					clt = new rpc::client(serverConn.ip, serverConn.port);
					clt->set_timeout(timeout);
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

	void Indexer::addNeighboor(conn_t neighbor) {
		neighbors.push_back(neighbor);
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

	uid_t Indexer::nextUID() { // https://stackoverflow.com/questions/19195183/how-to-properly-hash-the-custom-struct
		std::hash<logic_t> lh;
		std::hash<int> ih;

		LC++; // Increment clock

		uid_t next = lh(LC);
		next ^= ih(id) + 0x9e3779b9 + (next << 6) + (next >> 2);

		return next;
	}

	EntryResults Indexer::search(string query) {
		if (isServer)
			return database->search(query);
		return clt->call(k_Search, nextUID(), -1, query, 0).as<EntryResults>();
	}

	EntryResults Indexer::list() {
		if (isServer)
			return database->list();
		return clt->call(k_List, nextUID(), -1, 0).as<EntryResults>();
	}

	PeerResults Indexer::request(entryHash_t hash) {
		if (isServer)
			return database->request(hash);
		return clt->call(k_Request, nextUID(), -1, hash, 0).as<PeerResults>();
	}

	Database *Indexer::getDatabase() {
		return database;
	}
}
