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
#include <utility>
#include <string>
#include <unordered_set>

#include "rpc/server.h"

namespace Index {
	using std::string;
	using std::pair;
	using std::vector;
	using std::mutex;
	using std::unordered_set;
	using std::unordered_map;

	using entryHash_t = std::string;
		struct Peer;

	/**
	 * @brief struct conn_t represents connection info that is passed between client/server to represent connections
	*/
	struct conn_t {
		string ip;
		uint16_t port = 0;

		bool operator==(const struct conn_t &other) {
			return this->port == other.port && this->ip == other.ip;
		}

		string str() {
			return ip + ':' + std::to_string(port);
		}

		MSGPACK_DEFINE_ARRAY(ip, port);
	};

	/**
	 * @brief Information related to an origin server (leaf node)
	*/
	struct origin_t {
		int peerID = -1; // ID of the origin server
		conn_t conn; // Connection info for origin server
		bool master = false; // Whether this originated from the origin server
		size_t version = 0; // Version counter

		MSGPACK_DEFINE_ARRAY(peerID, conn, master, version);

		bool operator==(const struct origin_t &rhs) {
			return this->version == rhs.version && this->peerID == rhs.peerID && this->conn == rhs.conn;
		}

		bool matchesPeer(const Peer *peer) {
			return peer->connInfo == conn && peer->id == peerID;
		}

		origin_t(int peerID, conn_t conn, size_t version) : peerID { peerID }, conn { conn }, version { version } {
		}
		origin_t(Peer &peer, size_t version) : peerID { peer.id }, conn { peer.connInfo }, version { version } {
		}
	};

	/**
	 * @brief Template class that can have refrences to another datatype, used by Entry and Peer
	 * @tparam T The datatype to refrence
	*/
	template<typename T>
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

			MSGPACK_DEFINE_ARRAY(hash, name, origin, peers, firstIndexed);

			string firstIndexedString();
			string str();

			bool operator==(const searchEntry &rhs) const {
				return (this->hash == rhs.hash && this->name == rhs.name);
			}

			searchEntry &operator+=(const searchEntry &rhs) {
				// this->operator==(rhs) && 
				if (this->origin == rhs.origin) {
					peers += rhs.peers;
					if (rhs.firstIndexed < firstIndexed && rhs.firstIndexed > 0) {
						firstIndexed = rhs.firstIndexed;
					}
				} else {
					Log.e("searchEntry", "Attempted to concatenate two different search entries: %s:%s %s:%s", name, hash, rhs.name, rhs.hash);
					// TODO: invalidate file?
				}
				return *this;
			}
		};

		/**
		 * @brief Create a new Entry struct from the origin server
		 * @param name Name of this file entry
		 * @param hash The hash of this file entry
		 * @param peer The origin server peer struct
		 * @param version Version number of this file. defaults to 0
		 * @note this is should only be used internally
		*/
		Entry(string name, entryHash_t hash, Peer &peer, size_t version = 0) : name { name }, hash { hash } {
			isValid = true;
			origin = origin_t(peer, version);
		}

		/**
		 * @brief Create a new Entry struct
		 * @param name Name of this file entry
		 * @param hash The hash of this file entry
		 * @param origin The origin server information
		 * @note this is should only be used internally
		*/
		Entry(string name, entryHash_t hash, origin_t origin) : name { name }, hash { hash }, origin { origin } {
			isValid = true;
			origin.master = false; // set to false, as origin was not made using other constructor
		}

		//void update(string name, entryHash_t hash) {
		//	this->name = name;
		//	this->hash = hash;
		//}

		void invalidate() {
			TTR = 0; // force update
		}

		bool invalidated() {
			return TTR != -1 && TTR > std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		}

		bool operator==(const Entry &other) const {
			return hash == other.hash;
		}
	};

	/**
	 * @brief Peer represents a peer in the database, also refrences files that this peer has available
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

		bool operator==(const Peer &other) const {
			return id == other.id;
		}
	};

	using EntryResults = vector<Entry::searchEntry>;
	struct PeerResults {
		string fileName;
		std::vector<Peer::searchEntry> peers;

		MSGPACK_DEFINE_ARRAY(fileName, peers);

		PeerResults &operator+=(const PeerResults &rhs) {
			if (fileName == "" && rhs.fileName != "") {
				fileName = rhs.fileName;
			}
			peers.insert(peers.end(), rhs.peers.begin(), rhs.peers.end());
			return *this;
		}
	};
	using PeerResults = struct PeerResults;

	/**
	 * @brief basic database implementation that uses unordered_map to store peer and file entry information.
	*/
	class Database {
		std::mutex mutex; // TODO: use shared_mutex or optimize to allow registry and deregister run concurrently
		unordered_map<int, Peer *> peers {};
		unordered_map<entryHash_t, Entry *> entries {};

	public:

		/**
		 * @brief Register a new file to the database
		 * @param id The id of the peer to index
		 * @param connection The connection info of the peer to index
		 * @param entryName The name of the file
		 * @param hash The hash of the file
		 *
		 * @return If registering of the file was successful
		*/
		bool registry(int id, conn_t connection, string entryName, entryHash_t hash);		

		/**
		 * @brief Invalidate an existing file on the database
		 * @param hash hash of the file
		 *
		 * @return Peers that have the invalid entry
		*/
		unordered_set<Index::Peer> invalidate(entryHash_t hash);

		/**
		 * @brief Deregister a file from the database
		 * @param id The id of the peer deregistering
		 * @param connection The connection info of the peer deregistering
		 * @param hash The hash of the file to deregister
		 *
		 * @return If deregistering of the file was successful
		*/
		bool deregister(int id, conn_t connection, entryHash_t hash);

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

		/**
		 * @brief Request a specific file to be downloaded
		 * @param hash The hash of the file to download
		 * @return The list of peers that have this file
		*/
		PeerResults request(entryHash_t hash);
	};
}
