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
	typedef struct conn_t conn_t;
	struct Peer;

	struct conn_t {
		string ip;
		uint16_t port = 0;

		bool operator==(struct conn_t &other) {
			return this->port == other.port && this->ip == other.ip;
		}

		string str() {
			return ip + std::to_string(port);
		}

		MSGPACK_DEFINE_ARRAY(ip, port);
	};

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

	struct Entry : public Referer<Peer> {
		entryHash_t hash;
		string name;

		struct searchEntry {
			string hash;
			string name;
			size_t peers = 0;
			time_t firstIndexed = 0;

			MSGPACK_DEFINE_ARRAY(hash, name, peers, firstIndexed);

			string firstIndexedString();
			string str();
		};

		Entry(string name, entryHash_t hash) : name { name }, hash { hash } {
			isValid = true;
		}

		bool operator==(const Entry &other) const {
			return hash == other.hash;
		}
	};

	struct Peer : public Referer<Entry> {
		int id = -1;
		conn_t connInfo;

		struct searchEntry {
			int id = -1;
			conn_t connInfo;

			MSGPACK_DEFINE_ARRAY(id, connInfo);

			string str();
		};

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
	};
	using PeerResults = struct PeerResults;

	class Database {
		std::mutex mutex; // TODO: use shared_mutex or optimize to allow registry and deregister run concurrently
		unordered_map<int, Peer *> peers {};
		unordered_map<entryHash_t, Entry *> entries {};

	public:

		bool registry(int id, conn_t connection, string entryName, entryHash_t hash);

		bool deregister(int id, conn_t connection, entryHash_t hash);

		EntryResults search(string query);

		EntryResults list();

		PeerResults request(entryHash_t hash);

		void logEntries();

		void logSearch(string query);
	};
}
