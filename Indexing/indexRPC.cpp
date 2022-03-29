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
#include <future>

#include "rpc/server.h"
#include "rpc/this_handler.h"

#include "indexRPC.h"
#include "Exchanger.h"
#include <rpc/rpc_error.h>
#include <rpc/this_session.h>

namespace Index {

	static const int64_t timeout = 8000;

	Indexer::~Indexer() {
		if (srv != nullptr) {
			srv->stop();
			delete srv;
		}
		if (clt != nullptr)
			delete clt;
	}

	Indexer::Indexer(uint16_t sPort, int32_t totalSupers, bool pushing, bool pulling, bool all2all) : pushing { pushing }, pulling { pulling }, all2all { all2all }	{
		_TTL = totalSupers;
		srv = new rpc::server(sPort);
		database = new Database();
		serverConn.ip = "127.0.0.1"; // DNC
		serverConn.port = sPort;
		isServer = true;
		bindFunctions();
	}

	Indexer::Indexer(int id, uint16_t cPort, string sIP, uint16_t sPort, bool pushing, bool pulling, std::function<void(Index::Entry::searchEntry)> pullingListener) : pushing { pushing }, pulling { pulling }, pullingListener { pullingListener } {
		this->id = id;
		peerConn.ip = "127.0.0.1"; // Modified when connected to an indexing server
		peerConn.port = cPort;
		serverConn.ip = sIP;
		serverConn.port = sPort;
		isServer = false;
	}

	conn_t Indexer::getPeerConn() {
		return peerConn;
	};

	EntryResults searchEntries(uid_t uid, int32_t TTL, string query, conn_t neighbor, unsigned short _port) {
		try {
			Log.i("Indexer", "Propagating query %s %llu %u", k_Search.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			return clt.call(k_Search, uid, TTL, query, _port).as<EntryResults>();
		} catch (const std::exception &e) {
			Log.w("Indexer", "Neighbor error: %s:%u %s", neighbor.ip.data(), neighbor.port, e.what());
		}
		return EntryResults();
	}

	EntryResults listEntries(uid_t uid, int32_t TTL, conn_t neighbor, unsigned short _port) {
		try {
			Log.i("Indexer", "Propagating query %s %llu %u", k_List.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			return clt.call(k_List, uid, TTL, _port).as<EntryResults>();
		} catch (const std::exception &e) {
			Log.w("Indexer", "Neighbor error: %s", e.what());
		}
		return EntryResults();
	}

	PeerResults requestPeers(uid_t uid, int32_t TTL, string hash, conn_t neighbor, unsigned short _port) {
		try {
			Log.i("Indexer", "Propagating query %s %llu %u", k_Request.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			return clt.call(k_Request, uid, TTL, hash, _port).as<PeerResults>();
		} catch (const std::exception &e) {
			Log.w("Indexer", "Neighbor error: %s", e.what());
		}
		return PeerResults();
	}

	void invalidateEntry(uid_t uid, int32_t TTL, string hash, conn_t neighbor, unsigned short _port) {
		try {
			Log.i("Indexer", "Propagating query %s %llu %u", k_Invalidate.data(), uid, TTL);
			auto clt = rpc::client(neighbor.ip, neighbor.port);
			clt.set_timeout(timeout);
			clt.call(k_Invalidate, uid, TTL, hash, _port);
		} catch (const std::exception &e) {
			Log.w("Indexer", "Neighbor error: %s", e.what());
		}
	}

	void combineEntries(EntryResults &R, EntryResults &_R) {
		EntryResults _Ra;

		for (Entry::searchEntry &rs : _R) { // TODO: O(n^2)
			bool found = false;
			for (Entry::searchEntry &ls : R) {
				if (ls == rs && !rs.isMasterEntry()) { // Only merge if the entry being added is not a master one
					ls += rs; // TODO: time stamp is possibly wrong, due to race condition, not critical
					found = true;
					break; // There should at most be one exact duplicate per entry
				}
			}
			if (!found) {
				_Ra.push_back(rs);
			}
		}

		R.insert(R.begin(), _Ra.begin(), _Ra.end());
	}

	void splitResults(EntryResults &R, EntryResults &M) {
		EntryResults::iterator mit;

		for (mit = R.begin(); mit != R.end(); ) {
			Entry::searchEntry &r = *mit;
			if (r.origin.master) {
				M.push_back(r);
				mit = R.erase(mit);
			} else {
				mit++;
			}
		}
	}

	void Indexer::bindFunctions() {
		if (isServer) {
			srv->bind(k_Register, [&](int id, conn_t connection, string entryName, entryHash_t hash, Index::origin_t origin) -> bool { // TODO: if master, check connection info matches
				connection.ip = rpc::this_session().remoteAddr;
				return database->registry(id, connection, entryName, hash, origin);
					  });
			srv->bind(k_Deregister, [&](int id, conn_t connection, entryHash_t hash, bool master) -> bool { // TODO: if master, check connection info matches
				connection.ip = rpc::this_session().remoteAddr;
				return database->deregister(id, connection, hash, master); });
			srv->bind(k_GetOrigin, [&](entryHash_t hash) -> Index::origin_t {
				return database->getOrigin(hash); });						
			srv->bind(k_GetInvalidated, [&](int id) -> Index::EntryResults {
				return database->getInvalidated(id); 
			});
			srv->bind(k_UpdateTTR, [&](entryHash_t hash, time_t TTR) -> bool {
				return database->updateTTR(hash, TTR); });
			srv->bind(k_Ping, [&]() {return true; });

			// Propagated requests

			srv->bind(k_Invalidate, [&](uid_t uid, int32_t TTL, entryHash_t hash, unsigned short _port) -> bool {
				Log.d("Indexer", "Request Invalidation %d", TTL);
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					return false;
				}
				UIDs.insert(uid);
				uidMux.unlock();

				unordered_set<Peer> invalidated = database->invalidate(hash); // TODO: inform other leaf nodes here, _port initially is the port of the peer making the invalidation request

				if (pushing) {
					Log.i("Indexer", "Pushing invalidation: %s", hash.data());
					for (const Peer &peer : invalidated) {
						Exchanger::fileInvalidate(peer, hash);
					}

					if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
						if (all2all) {
							TTL = 0; // propagated calls will not propagate any further

							string _addr = rpc::this_session().remoteAddr;
							vector<std::future<void>> results;

							for (conn_t neighbor : neighbors) {
								if (neighbor.ip != _addr || neighbor.port != _port)
									results.push_back(std::async(invalidateEntry, uid, TTL, hash, neighbor, serverConn.port));
							}

							for (std::future<void> &result : results) {
								result.wait();
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

						invalidateEntry(uid, TTL, hash, neighbor, serverConn.port);
					} else {
						Log.i("Indexer", "TTL finished %llu", uid);
					}
				}

				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return invalidated.size() > 0;
					  });

			srv->bind(k_Search, [&](uid_t uid, int32_t TTL, string query, unsigned short _port) -> EntryResults {
				Log.d("Indexer", "Request Search %d", TTL);
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					return EntryResults();
				}
				UIDs.insert(uid);
				uidMux.unlock();

				EntryResults R = database->search(query);

				bool inital = false;

				if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
					if (all2all) {
						TTL = 0; // propagated calls will not propagate any further

						string _addr = rpc::this_session().remoteAddr;
						vector<std::future<EntryResults>> results;
						EntryResults _R;

						for (conn_t neighbor : neighbors) {
							if (neighbor.ip != _addr || neighbor.port != _port)
								results.push_back(std::async(searchEntries, uid, TTL, query, neighbor, serverConn.port));
						}

						for (std::future<EntryResults> &result : results) {
							EntryResults _Ra = result.get();
							_R.insert(_R.begin(), _Ra.begin(), _Ra.end());
						}

						combineEntries(R, _R);
					} else {
						TTL = _TTL;
					}

					inital = true;
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

					auto _Ra = searchEntries(uid, TTL, query, neighbor, serverConn.port);
					combineEntries(R, _Ra);
				} else {
					Log.i("Indexer", "TTL finished %llu", uid);
				}

				if (inital) {
					EntryResults M;
					splitResults(R, M);
					EntryResults F;
					Index::combineResults(M, R, F, false);
					for (Entry::searchEntry &entry : F)
						entry.peers--; // subtract one to remove origin duplicate
					R = F;
				}

				Log.d("Indexer", "Returning results");
				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return R;
					  });

			srv->bind(k_List, [&](uid_t uid, int32_t TTL, unsigned short _port) -> EntryResults {
				Log.d("Indexer", "Request Listing %d", TTL);
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					return EntryResults();
				}
				UIDs.insert(uid);
				uidMux.unlock();

				bool inital = false;

				EntryResults R = database->list();

				if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
					if (all2all) {
						TTL = 0; // propagated calls will not propagate any further

						string _addr = rpc::this_session().remoteAddr;
						vector<std::future<EntryResults>> results;
						EntryResults _R;

						for (conn_t neighbor : neighbors) {
							if (neighbor.ip != _addr || neighbor.port != _port)
								results.push_back(std::async(listEntries, uid, TTL, neighbor, serverConn.port));
						}

						for (std::future<EntryResults> &result : results) {
							EntryResults _Ra = result.get();
							_R.insert(_R.begin(), _Ra.begin(), _Ra.end());
						}

						combineEntries(R, _R);

					} else {
						TTL = _TTL;
					}

					inital = true;
				}

				TTL--;

				if (TTL > 0) {
					// Get connection info of this neighbor/peer to ensure we don't propagate backwards
					string _addr = rpc::this_session().remoteAddr;

					conn_t neighbor;

					for (conn_t &n : neighbors) {
						if (n.ip != _addr || n.port != _port) { // Either differing attribute means we aren't propagating to the same super peer
							neighbor = n;
							break;
						}
					}

					auto _R = listEntries(uid, TTL, neighbor, serverConn.port);
					combineEntries(R, _R);
				} else {
					Log.i("Indexer", "TTL finished %llu", uid);
				}

				if (inital) {
					EntryResults M;
					splitResults(R, M);
					EntryResults F;
					Index::combineResults(M, R, F, false);
					for (Entry::searchEntry &entry : F)
						entry.peers--; // subtract one to remove origin duplicate
					R = F;
				}

				Log.d("Indexer", "Returning results");
				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return R;
					  });

			srv->bind(k_Request, [&](uid_t uid, int32_t TTL, entryHash_t hash, unsigned short _port) -> PeerResults {
				Log.d("Indexer", "Request File %d", TTL);
				uidMux.lock();
				if (UIDs.contains(uid)) {
					uidMux.unlock();
					Log.e("Indexer", "UID already in progress %llu", uid);
					rpc::this_handler().respond_error("UID already in progress");
					return PeerResults();
				}
				UIDs.insert(uid);
				uidMux.unlock();

				auto R = database->request(hash);

				if (TTL == -1) { // -1 means an actual peer is making the request, initialize to total super peers
					if (all2all) {
						TTL = 0; // propagated calls will not propagate any further

						string _addr = rpc::this_session().remoteAddr;
						vector<std::future<PeerResults>> results;
						EntryResults _R;

						for (conn_t neighbor : neighbors) {
							if (neighbor.ip != _addr || neighbor.port != _port)
								results.push_back(std::async(requestPeers, uid, TTL, hash, neighbor, serverConn.port));
						}

						for (std::future<PeerResults> &result : results) {
							R += result.get();
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

					for (conn_t &n : neighbors) {
						if (n.ip != _addr || n.port != _port) { // Either differing attribute means we aren't propagating to the same super peer
							neighbor = n;
							break;
						}
					}

					R += requestPeers(uid, TTL, hash, neighbor, serverConn.port);
				}

				UIDs.erase(uid); // Potential to run query again? clear at a later time?
				return R;
					  });
		}
	}

	void Indexer::stop() {
		running = false;
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

	void Indexer::pullingThread(time_t delay) {
		try {
			while (running) {
				std::this_thread::sleep_for(std::chrono::seconds(delay));
				EntryResults results = clt->call(k_GetInvalidated, id).as<EntryResults>();
				for (Index::Entry::searchEntry &r : results) {
					pullingListener(r);
				}
			}
		} catch (std::exception &e) {
			Log.f("Indexer", "Pulling thread exception: %s\n", e.what());
		}

	}

	void Indexer::start(time_t delay) {
		running = true;
		if (isServer) {
			Log.i("Indexer", "Running Server");
			srv->async_run(std::thread::hardware_concurrency() * 2);
		} else {
			if (pulling) {
				Log.i("Indexer", "Running client pulling thread");
				std::thread(&Indexer::pullingThread, this, delay).detach();
			}
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

	Index::origin_t Indexer::getOrigin(entryHash_t hash) {
		if (isServer) {
			return database->getOrigin(hash);
		}
		return clt->call(k_GetOrigin, hash).as<Index::origin_t>();
	}

	bool Indexer::updateTTR(Index::entryHash_t hash, time_t TTR) {
		if (isServer) {
			return database->updateTTR(hash, TTR);
		}
		return clt->call(k_UpdateTTR, hash, TTR).as<bool>();
	}

	bool Indexer::registry(string entryName, entryHash_t hash, Index::origin_t origin) { // TODO: disallow zero byte files
		if (isServer)
			return false;
		return clt->call(k_Register, id, peerConn, entryName, hash, origin).as<bool>();
	}

	bool Indexer::deregister(entryHash_t hash, bool master) {
		if (isServer)
			return false;
		return clt->call(k_Deregister, id, peerConn, hash, master).as<bool>();
	}

	bool Indexer::invalidate(entryHash_t hash) {
		if (isServer)
			return false;
		try {
			return clt->call(k_Invalidate, nextUID(), -1, hash, peerConn.port).as<bool>();
		} catch (const std::exception &e) {
			Log.e("Invalidate", "Error running query: %s", e.what());
		}
		return false;
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
