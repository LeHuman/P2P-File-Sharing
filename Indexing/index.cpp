/**
 * @file index.cpp
 * @author IR
 * @brief Source code for the Index module
 * @version 0.1
 * @date 2022-02-20
 *
 * @copyright Copyright (c) 2022
 *
 */

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

	origin_t::origin_t(int peerID, conn_t conn, bool master) : peerID { peerID }, conn { conn }, master { master } {}
	origin_t::origin_t(Peer *peer) : peerID { peer->id }, conn { peer->connInfo } {
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

	unordered_set<Peer> Database::invalidate(entryHash_t hash) {
		mutex.lock();
		unordered_map<entryHash_t, Entry *>::const_iterator gotE = remotes.find(hash);

		Entry *e = NULL;

		unordered_set<Peer> relatedPeers;

		if (gotE == remotes.end()) {
			Log.w("Invalidate", "Entry not found: %s", hash.data());
		} else {
			e = remotes[hash];
		}

		if (e == NULL || !e->valid()) {
			Log.e("Invalidate", "Entry was not found or is invalid\n\thash: %s", hash.data());
			mutex.unlock();
			return relatedPeers;
		}

		e->invalidate();

		Peer *M = nullptr;

		if (origins.contains(hash)) {
			origins[hash]->invalidate();
			M = *begin(origins[hash]->refrences);
		}

		for (Index::Peer *peer : e->refrences) {
			if (M == nullptr || M->id != peer->id) {
				relatedPeers.insert(*peer);
			}
		}

		mutex.unlock();

		Log("Invalidate", "Invalidate\n\tname: %s\n\thash: %s\n", e->name.data(), hash.data());

		return relatedPeers;
	}

	bool Database::registry(int id, conn_t connection, string entryName, entryHash_t hash, origin_t origin) {
		mutex.lock();

		unordered_map<entryHash_t, Entry *>::const_iterator gotE = remotes.find(hash); // TODO: remove need for mutex here by ensuring matching registry call is not being run
		unordered_map<int, Peer *>::const_iterator gotP = peers.find(id);

		bool nameExists = false;

		if (gotP == peers.end()) {
			peers[id] = new Peer(id, connection);
		}
		Peer *p = peers[id];

		if (origin.master) {
			if (origins.contains(hash)) {
				Log.w("Registry", "Origin file already locally exists\n\tID: %d\n\thash: %s", id, hash.data());
			} else {
				Log.i("Registry", "New Origin\n\tID: %d\n\tname: %s\n\thash: %s\n", id, entryName.c_str(), hash.data());
				origins[hash] = new Entry(entryName, hash, origin);
				origins[hash]->add(p);
				origins[hash]->origin.master = true;
			}
		}

		if (gotE == remotes.end()) {
			remotes[hash] = new Entry(entryName, hash, origin);
			if (origin.peerID == -1) {
				Log.f("Registry", "Entry has invalid origin: %s:%s", entryName.data(), hash.data());
			}
		} else if (gotE->second->name != entryName) {
			Log.w("Registry", "Entry hash already exists, using indexed entry name");
			nameExists = true;
		}
		Entry *e = remotes[hash];

		if (!e->valid() || !p->valid()) {
			Log.e("Registry", "Referrers invalid before addition\n\tID: %d\n\thash: %s", id, hash.data());
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
				Log.w("Registry", "Peer and Entry already refrence each other\n\tID: %d\n\thash: %s", id, hash.data());
				return true;
		}

		Log("Registry", "New\n\tID: %d\n\tname: %s%s\n\thash: %s\n", id, entryName.c_str(), (nameExists ? " -> " + saveName : "").c_str(), hash.data());

		return true;
	}

	bool Database::deregister(int id, conn_t connection, entryHash_t hash, bool master) {
		mutex.lock();

		unordered_map<entryHash_t, Entry *>::const_iterator gotE = remotes.find(hash);
		unordered_map<int, Peer *>::const_iterator gotP = peers.find(id);

		if (master) {
			if (!origins.contains(hash)) {
				Log.e("Registry", "Origin file does not exist\n\tID: %d\n\thash: %s", id, hash.data());
			} else {
				Entry *e = origins[hash];
				origins.erase(hash);
				delete e;
				// TODO: propagate deletion?
			}
		}

		Entry *e = NULL;
		Peer *p = NULL;

		if (gotE == remotes.end()) {
			Log.w("Deregister", "Entry not found: %s", hash.data());
		} else {
			e = remotes[hash];
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

		int removed = 0;

		if (!e->remove(p)) {
			removed += 1;
		}

		if (!p->remove(e)) {
			removed -= 2;
		}

		if (!e->valid()) {
			//e->mutex.lock();
			remotes.erase(e->hash);
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

		switch (removed) {
			case 1:
				Log.w("Deregister", "Entry did not refrence peer\n\tID: %d\n\thash: %s", id, hash.data());
				break;
			case -2:
				Log.w("Deregister", "Peer did not refrence entry\n\tID: %d\n\thash: %s", id, hash.data());
				break;
			case -1:
				Log.w("Deregister", "Peer and Entry already don't reference each other\n\tID: %d\n\thash: %s", id, hash.data());
				return true;
		}

		if (removed != 0) {
			Log.e("Deregister", "Mutual removal was not achieved\n\tID: %d\n\thash: %s", id, hash.data());
		}

		Log("Deregister", "New\n\tID: %d\n\tname: %s\n\thash: %s\n", id, saveName.c_str(), hash.data());

		return removed == 0;
	}

	void combineResults(EntryResults &origins, EntryResults &remotes, EntryResults &results, bool leftovers) {
		for (Entry::searchEntry &o : origins) {
			EntryResults::iterator mit;

			for (mit = remotes.begin(); mit != remotes.end(); ) {
				Entry::searchEntry &r = *mit;
				if (o.name == r.name) {
					if (o.hash == r.hash) { // TODO: match version number?
						o += r;
					}
					mit = remotes.erase(mit);
				} else {
					mit++;
				}
			}
			results.push_back(o);
		}
		if (leftovers) {
			results.insert(results.end(), remotes.begin(), remotes.end());
		}
	}

	EntryResults Database::list() {
		EntryResults results;
		EntryResults _origins;
		EntryResults _remotes;

		Log("List", "Listing all files entries");

		mutex.lock(); // TODO: lock individual Entry mutex instead, must consider entry being deleted
		for (pair<entryHash_t, Entry *> entry : remotes) {
			Entry *val = entry.second;
			_remotes.emplace_back(val->hash, val->name, val->origin, val->refrenceCount(), val->firstIndexed);
		}
		for (pair<entryHash_t, Entry *> entry : origins) {
			Entry *val = entry.second;
			_origins.emplace_back(val->hash, val->name, val->origin, val->refrenceCount(), val->firstIndexed);
		}
		mutex.unlock();

		if (_remotes.empty()) {
			Log.w("List", "Query resulted empty");
			return results;
		}

		combineResults(_origins, _remotes, results);

		return results;
	}

	void toLower(std::string &str) {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	}

	EntryResults Database::search(string query) {
		EntryResults results;
		EntryResults _origins;
		EntryResults _remotes;

		Log("Search", "Searching for query: %s", query.data());

		string search_v(query);

		toLower(search_v);

		mutex.lock();
		for (pair<entryHash_t, Entry *> entry : remotes) {
			string lowerName(entry.second->name);
			toLower(lowerName);

			if (lowerName.find(search_v) != string::npos) {
				Entry *val = entry.second;
				_remotes.emplace_back(val->hash, val->name, val->origin, val->refrenceCount(), val->firstIndexed);
			}
		}

		for (pair<entryHash_t, Entry *> entry : origins) {
			string lowerName(entry.second->name);
			toLower(lowerName);

			if (lowerName.find(search_v) != string::npos) {
				Entry *val = entry.second;
				_origins.emplace_back(val->hash, val->name, val->origin, val->refrenceCount(), val->firstIndexed);
			}
		}
		mutex.unlock();

		if (_remotes.empty()) {
			Log.w("Search", "Query resulted empty: %s", query.data());
			return results;
		}

		combineResults(_origins, _remotes, results);

		return results;
	}

	PeerResults Database::request(entryHash_t hash) {
		PeerResults results;

		Log("Request", "Searching for hash: %s", hash.data());

		mutex.lock();
		unordered_map<entryHash_t, Entry *>::const_iterator got = remotes.find(hash);

		if (got == remotes.end()) {
			mutex.unlock();
			results.fileName = "";
			Log.w("Request", "No entries found with hash: %s", hash.data());
		} else {
			results.fileName = got->second->name;
			Log.i("Request", "Entry found\n\tname: %s\n\thash: %s", results.fileName.c_str(), hash.data());

			for (const Peer *peer : got->second->refrences) {
				results.peers.emplace_back(peer->id, peer->connInfo);
			}
			mutex.unlock();
		}

		return results;
	}

	origin_t Database::getOrigin(entryHash_t hash) {
		std::lock_guard<std::mutex> lck(mutex);
		if (origins.contains(hash)) {
			return origins[hash]->origin;
		}
		if (remotes.contains(hash)) {
			return remotes[hash]->origin;
		}

		//for (auto remote : remotes) {
		//	if (remote.second->name == hash) {
		//		return remote.second->origin;
		//	}
		//}

		Log.e("getOrigin", "Unable to get origin: %s", hash.data());
		return origin_t();
	}

	bool Database::updateTTR(entryHash_t hash, time_t TTR) {
		std::lock_guard<std::mutex> lck(mutex);
		if (remotes.contains(hash)) {
			remotes[hash]->setTTR(TTR);
			return true;
		}
		Log.e("updateTTR", "unable to update TTR: %s", hash.data());
		return false;
	}
}
