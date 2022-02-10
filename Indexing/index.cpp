#include <algorithm>
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <forward_list>
#include <unordered_map>
#include <cstdarg>
#include <mutex>

#include "Log.h"
#include "index.h"

namespace Index {
	using std::unordered_map;

	static std::mutex mutex; // TODO: use shared_mutex or optimize to allow registry and deregister run concurrently

	unordered_map<int, Peer *> peers {};
	unordered_map<entryHash_t, Entry *> entries {};

	bool registry(int id, conn_t connection, string entryName, entryHash_t hash) {
		mutex.lock();

		unordered_map<entryHash_t, Entry *>::const_iterator gotE = entries.find(hash); // TODO: remove need for mutex here by ensuring matching registry call is not being run
		unordered_map<int, Peer *>::const_iterator gotP = peers.find(id);

		bool nameExists = false;

		if (gotE == entries.end()) {
			entries[hash] = new Entry(entryName, hash);
		} else if (gotE->second->name != entryName) {
			Log.w("Registry", "Entry hash already exists, using indexed entry name");
			nameExists = true;
		}

		if (gotP == peers.end()) {
			peers[id] = new Peer(id, connection);
		}

		Entry *e = entries[hash];
		Peer *p = peers[id];

		if (!e->valid() || !p->valid()) {
			Log.e("Registry", "Referers invalid before addition\n\tID: %d\n\thash: %s", id, hash.data());
			mutex.unlock();
			return false;
		}

		string saveName = e->name;

		/*mutex.unlock();*/

		int added = 0;

		if (!e->add(p)) {
			added += 1;
		}

		if (!p->add(e)) {
			added -= 2;
		}

		mutex.unlock();

		switch (added) {
			case 1:
				Log.w("Registry", "Entry already refrences peer\n\tID: %d\n\thash: %s", id, hash.data());
				break;
			case -2:
				Log.w("Registry", "Peer already refrences entry\n\tID: %d\n\thash: %s", id, hash.data());
				break;
			case -1:
				return false;
		}

		Log("Registry", "New\n\tID: %d\n\tname: %s%s\n\thash: %s\n", id, entryName.c_str(), (nameExists ? " -> " + saveName : "").c_str(), hash.data());

		return true;
	}

	bool deregister(int id, conn_t connection, entryHash_t hash) {
		mutex.lock();

		unordered_map<entryHash_t, Entry *>::const_iterator gotE = entries.find(hash);
		unordered_map<int, Peer *>::const_iterator gotP = peers.find(id);

		Entry *e = NULL;
		Peer *p = NULL;

		if (gotE == entries.end()) {
			Log.w("Deregister", "Entry not found: %s", hash.data());
		} else {
			e = entries[hash];
		}

		if (gotP == peers.end()) {
			Log.w("Deregister", "Peer not found: %d", id);
		} else {
			p = peers[id];
		}

		if (e == NULL || p == NULL) {
			Log.e("Deregister", "Referers were not found\n\tID: %d\n\thash: %s", id, hash.data());
			mutex.unlock();
			return false;
		}

		if (!e->valid() || !p->valid()) {
			Log.e("Deregister", "Referers invalid before removal\n\tID: %d\n\thash: %s", id, hash.data());
			mutex.unlock();
			return false;
		}

		if (p->connInfo != connection) {
			Log.e("Deregister", "Mismatching connection info\n\tID: %d\n\thash: %s", id, connection.str().data(), hash.data());
			mutex.unlock();
			return false;
		}

		string saveName = e->name;

		/*mutex.unlock();*/

		bool removed = true;

		removed &= e->remove(p);
		removed &= p->remove(e);

		if (!e->valid()) {
			//e->mutex.lock();
			entries.erase(e->hash);
			Log("Deregister", "Erase entry\n\tname: %s\n\thash: %s", saveName.c_str(), hash.data());
			delete e;
		}

		if (!p->valid()) {
			//p->mutex.lock();
			int _id = p->id;
			peers.erase(_id);
			Log("Deregister", "Erase peer\n\tID: %d", _id);
			delete p;
		}

		mutex.unlock();

		if (!removed) {
			Log.e("Deregister", "Mutual removal was not achieved\n\tID: %d\n\thash: %s", id, hash.data());
		}

		Log("Deregister", "New\n\tID: %d\n\tname: %s\n\thash: %s\n", id, saveName.c_str(), hash.data());

		return removed;
	}

	inline std::tm localtime_xp(time_t timer) {
		std::tm bt {};
#if defined(__unix__)
		localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
		localtime_s(&bt, &timer);
#else
		static std::mutex mtx;
		std::lock_guard<std::mutex> lock(mtx);
		bt = *std::localtime(&timer);
#endif
		return bt;
	}

	string Entry::searchEntry::firstIndexedString() {
		static char buf[256];
		struct std::tm tm = localtime_xp(firstIndexed);
		if (std::strftime(buf, sizeof(buf), "%d/%m/%Y %T", &tm))
			return std::string { buf };
		throw std::runtime_error("Failed to get current date as string");
	}

	string Entry::searchEntry::str() {
		std::ostringstream ss;
		ss << "Hash: " << hash << "\nAdded on " << firstIndexedString() << "\n\tName: " << name << "\n\tPeers: " << peers;
		return ss.str();
	}

	string Peer::searchEntry::str() {
		std::ostringstream ss;
		ss << "[" << id << "] " << connInfo.ip << ":" << connInfo.port;
		return ss.str();
	}

	EntryResults list() {
		EntryResults results;

		mutex.lock(); // TODO: lock individual Entry mutex instead, must consider entry being deleted
		for (pair<entryHash_t, Entry *> entry : entries) {
			string name = entry.second->name;
			string hash = entry.second->hash;
			results.emplace_back(hash, name, entry.second->refrenceCount(), entry.second->firstIndexed);
		}
		mutex.unlock();

		return results;
	}

	EntryResults search(string query) {
		EntryResults results;

		string search_v(query);

		std::transform(search_v.begin(), search_v.end(), search_v.begin(), [](unsigned char c) { return (char)std::tolower(c); });

		mutex.lock();
		for (pair<entryHash_t, Entry *> entry : entries) {
			string lowerName(entry.second->name);
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) { return (char)std::tolower(c); });

			if (lowerName.find(search_v) != string::npos) {
				string name = entry.second->name;
				string hash = entry.second->hash;
				results.emplace_back(hash, name, entry.second->refrenceCount(), entry.second->firstIndexed);
			}
		}
		mutex.unlock();

		if (results.empty()) {
			Log.w("Search", "Query resulted empty: %s", query.data());
		}

		return results;
	}

	PeerResults request(entryHash_t hash) {
		PeerResults results;

		mutex.lock();
		unordered_map<entryHash_t, Entry *>::const_iterator got = entries.find(hash);

		if (got == entries.end()) {
			mutex.unlock();
			Log.w("Request", "No entries found with hash: %s", hash.data());
		} else {
			Log.i("Request", "Entry found\n\tname: %s\n\thash: %s", got->second->name.c_str(), hash.data());

			for (const Peer *peer : got->second->refrences) {
				results.emplace_back(peer->id, peer->connInfo);
			}
			mutex.unlock();
		}

		return results;
	}

	void logEntries() {
		for (Index::Entry::searchEntry entry : list()) {
			Log.i("Lister", "Name:%s\n\tpeers:%lld\n\ti:%s\n\tH:%s", entry.name.c_str(), entry.peers, entry.firstIndexedString().data(), entry.hash.data());
		}
	}

	void logSearch(string query) {
		for (Index::Entry::searchEntry entry : search(query)) {
			Log.i("Searcher", "Name:%s\n\tpeers:%lld\n\ti:%s\n\tH:%s", entry.name.c_str(), entry.peers, entry.firstIndexedString().data(), entry.hash.data());
		}
	}
}
