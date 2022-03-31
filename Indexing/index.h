/**
 * @file index.h
 * @author IR
 * @brief Module that deals with indexing files
 * @version 0.1
 * @date 2022-02-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>

#include "Log.h"
#include "rpc/server.h"

namespace Index {
	using std::mutex;
	using std::pair;
	using std::string;
	using std::unordered_map;
	using std::unordered_set;
	using std::vector;
	struct Peer;

	using entryHash_t = std::string;

	/**
	 * @brief struct conn_t represents connection info that is passed between client/server to represent connections
	 */
	struct conn_t {
		string ip;
		uint16_t port = 0;

		bool operator==(struct conn_t &other) {
			return this->port == other.port && this->ip == other.ip;
		}

		string str() {
			return ip + ':' + std::to_string(port);
		}

		MSGPACK_DEFINE_ARRAY(ip, port);
	};
/*--------- start change ----------*/
	/**
	 * @brief Information related to an origin server (leaf node)
	 */
	struct origin_t {
		int peerID = -1;     // ID of the origin server
		conn_t conn;         // Connection info for origin server
		bool master = false; // Whether this originated from the origin server

		MSGPACK_DEFINE_ARRAY(peerID, conn, master);

		bool operator==(struct origin_t &rhs) {
			return this->peerID == rhs.peerID && this->conn == rhs.conn;
		}

		origin_t(int peerID, conn_t conn, bool master);
		origin_t(Peer *peer);
		origin_t() {};

		string str() {
			return std::to_string(peerID) + ':' + conn.str() + ':' + std::to_string(master);
		}
	};
/*--------- end change ----------*/
	/**
	 * @brief Template class that can have refrences to another datatype, used by Entry and Peer
	 * @tparam T The datatype to refrence
	 */
	template <typename T>
	class Referer {
	protected:
		bool isValid = false;

	public:
		std::mutex mutex;
		unordered_set<T *> refrences {};
		const time_t firstIndexed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		bool add(T *obj) {
			mutex.lock();
			bool added = refrences.insert(obj).second;
			mutex.unlock();
			return added;
		}

		bool remove(T *obj) {
			mutex.lock();
			bool erased = (bool)refrences.erase(obj);
			isValid = refrenceCount() > 0;
			mutex.unlock();
			return erased;
		}

		size_t refrenceCount() {
			return refrences.size();
		}

		bool valid() {
			return isValid;
		}
	};

/*--------- start change ----------*/

	/**
	 * @brief Entry represents a file entry in the database, also references Peers that have this file available
	 */
	struct Entry : public Referer<Peer> {
		entryHash_t hash;
		string name;
		origin_t origin;
		time_t TTR = -1; // Time to refresh, -1 means no invalidating,

		/**
		 * @brief Entry::searchEntry is used to pass information over RPC, related to entries
		 */
		struct searchEntry {
			entryHash_t hash;
			string name;
			origin_t origin;
			size_t peers = 0;
			time_t firstIndexed = 0;
			time_t TTR = -1;
			float icount = 0;

			MSGPACK_DEFINE_ARRAY(hash, name, origin, peers, firstIndexed, TTR, icount);

			searchEntry(){}
			searchEntry(entryHash_t hash, string name, origin_t origin, size_t peers, time_t firstIndexed, time_t TTR) : hash { hash }, name { name }, origin { origin }, peers { peers }, firstIndexed { firstIndexed }, TTR { TTR } {
			}

			string firstIndexedString();
			string str();

			bool operator==(searchEntry &rhs) {
				return (this->hash == rhs.hash && this->name == rhs.name && this->origin == rhs.origin);
			}

			bool isMasterEntry() {
				return origin.master;
			}

			searchEntry &operator+=(searchEntry &rhs) {
				// this->operator==(rhs) &&
				// if (this->origin == rhs.origin) {
				peers += rhs.peers;
				icount += rhs.icount;
				if (rhs.firstIndexed < firstIndexed && rhs.firstIndexed > 0) {
					firstIndexed = rhs.firstIndexed;
				}
				//} else {
				//	Log.w("searchEntry", "Attempted to concatenate two different search entries: %s:%s:%s %s:%s:%s", name.data(), hash.data(), this->origin.str().data(), rhs.name.data(), rhs.hash.data(), rhs.origin.str().data());
				//	// TODO: invalidate file?
				//}
				return *this;
			}
		};

		/**
		 * @brief Create a new Entry struct
		 * @param name Name of this file entry
		 * @param hash The hash of this file entry
		 * @param peer The origin server peer struct
		 * @param version Version number of this file. defaults to 0
		 * @note this is should only be used internally
		 */
		Entry(string name, entryHash_t hash, origin_t origin) : name { name }, hash { hash } {
			isValid = true;
			this->origin = origin;
		}

		Entry(const Index::Entry &e) : name { e.name }, hash { e.hash } {
			isValid = true;
			origin = e.origin;
		}

		void setTTR(time_t TTR) {
			this->TTR = TTR;
			Log.d("Entry", "TTR Updated: %s:%lld", name.data(), TTR);
		}

		void invalidate() {
			TTR = 0; // force update
		}

		bool invalidated() {
			return TTR < std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); // TODO: don't check actual time each time
		}

		bool operator==(const Entry &other) const {
			return hash == other.hash;
		}
	};
/*--------- end change ----------*/
	/**
	 * @brief Peer represents a peer in the database, also references files that this peer has available
	 */
	struct Peer : public Referer<Entry> {
		int id = -1;
		conn_t connInfo;

		/**
		 * @brief Peer::searchEntry is used to pass information over RPC, related to peers
		 */
		struct searchEntry {
			int id = -1;
			conn_t connInfo;

			MSGPACK_DEFINE_ARRAY(id, connInfo);

			string str();
		};

		/**
		 * @brief Create a new Peer struct
		 * @param id The unique id of this peer
		 * @param connInfo The connection info for this peer
		 * @note this is should only be used internally
		 */
		Peer(int id, conn_t connInfo) : id { id }, connInfo { connInfo } {
			isValid = true;
		}

		Peer(const Index::Peer &p) : id { p.id }, connInfo { p.connInfo } {
			isValid = true;
		}

		bool operator==(const Peer &other) const {
			return id == other.id;
		}
	};

	using EntryResults = vector<Entry::searchEntry>;
	struct PeerResults {
		string fileName;
		std::vector<Peer::searchEntry> peers;

		MSGPACK_DEFINE_ARRAY(fileName, peers);
/*--------- start change ----------*/
		PeerResults() {}

		PeerResults(string fileName, std::vector<Peer::searchEntry> peers) : fileName { fileName }, peers { peers } {
		}

		PeerResults(string fileName, int id, conn_t connInfo) : fileName { fileName } {
			peers.emplace_back(id, connInfo);
		}
/*--------- end change ----------*/
		PeerResults &operator+=(const PeerResults &rhs) {
			if (fileName == "" && rhs.fileName != "") {
				fileName = rhs.fileName;
			}
			peers.insert(peers.end(), rhs.peers.begin(), rhs.peers.end());
			return *this;
		}
	};
	using PeerResults = struct PeerResults;

	void combineResults(EntryResults &origins, EntryResults &remotes, EntryResults &results, bool leftovers = true);

	/**
	 * @brief basic database implementation that uses unordered_map to store peer and file entry information.
	 */
	class Database {
		std::mutex mutex; // TODO: use shared_mutex or optimize to allow registry and deregister run concurrently
		unordered_map<int, Peer *> peers {};
		unordered_map<entryHash_t, Entry *> remotes {};
		unordered_map<entryHash_t, Entry *> origins {};

	public:
    /*--------- start change ----------*/
		/**
		 * @brief Register a new file to the database
		 * @param id The id of the peer to index
		 * @param connection The connection info of the peer to index
		 * @param entryName The name of the file
		 * @param hash The hash of the file
		 * @param origin information about both the request and file
		 *
		 * @return If registering of the file was successful
		 */
		bool registry(int id, conn_t connection, string entryName, entryHash_t hash, origin_t origin);

		/**
		 * @brief Invalidate an existing file on the database
		 * @param hash hash of the file
		 *
		 * @return Peers that have the invalid entry
		 */
		unordered_set<Index::Peer> invalidate(entryHash_t hash);
/*--------- end change ----------*/
		/**
		 * @brief Deregister a file from the database
		 * @param id The id of the peer deregistering
		 * @param connection The connection info of the peer deregistering
		 * @param hash The hash of the file to deregister
		 * @param master True if origin server is running command
		 *
		 * @return If deregistering of the file was successful
		 */
		bool deregister(int id, conn_t connection, entryHash_t hash, bool master);

		/**
		 * @brief Search for a file entry by it's name
		 * @param query The substring to search for
		 * @return The vector of entries that were found
		 */
		EntryResults search(string query);

		/**
		 * @brief List every file on the database
		 * @return The vector of every entry
		 */
		EntryResults list();
/*--------- start change ----------*/
		/**
		 * @brief Request a specific file to be downloaded
		 * @param hash The hash of the file to download
		 * @return The list of peers that have this file
		 */
		PeerResults request(entryHash_t hash);

		EntryResults getInvalidated(int id);

		origin_t getOrigin(entryHash_t hash);

		bool updateTTR(entryHash_t hash, time_t TTR);
	};
} // namespace Index

namespace std {
	template <>
	struct std::hash<Index::Peer> {
		std::size_t operator()(const Index::Peer &p) const noexcept {
			return p.id;
		}
	};
} // namespace std

/*--------- end change ----------*/