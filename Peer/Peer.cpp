/**
 * @file Peer.cpp
 * @author IR
 * @brief Source code for the peer module
 * @version 0.1
 * @date 2022-02-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "Parsers.h"
#include "Folder.h"
#include "Peer.h"

using std::string;

static const string _ID = "Peer";

void Peer::registerFile(string fileName, Index::entryHash_t hash, Index::origin_t origin) {
	bool registered = indexer->registry(fileName, hash, origin); // TODO: version number
	if (registered) {
		Log.i(_ID, "Registered hash: %s", fileName.data());
	} else {
		Log.e(_ID, "Unable to register hash: %s", fileName.data());
	}
}

void Peer::deregisterFile(Index::entryHash_t hash, bool master) {
	bool deregistered = indexer->deregister(hash, master);
	if (deregistered) {
		Log.i(_ID, "Deregistered hash: %s", hash.data());
	} else {
		Log.e(_ID, "Unable to deregister hash: %s", hash.data());
	}
}
/*--------- start change ----------*/
void Peer::invalidateFile(Index::entryHash_t hash) {
	bool invalidated = indexer->invalidate(hash);
	if (invalidated) {
		Log.i(_ID, "Invalidated hash: %s", hash.data());
	} else {
		Log.w(_ID, "Unable to invalidate hash or no peers to locally invalidate: %s", hash.data());
	}
}

void Peer::originFolderListener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	switch (status) {
		case Util::File::Status::created:
			exchanger->addLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, Index::origin_t(id, indexer->getPeerConn(), true)); // TODO: run with master bit
			break;
		case Util::File::Status::erased:
			deregisterFile(file.hash, true);
			exchanger->removeLocalFile(file);
			break;
		case Util::File::Status::modified:
			if (file.prehash == file.hash) {
				Log.w(_ID, "File modified retained hash, not updating: %s", file.hash.data());
				break;
			}
			deregisterFile(file.prehash, true);
			exchanger->updateLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, Index::origin_t(id, indexer->getPeerConn(), true));
			Log.d("originFolderListener", "Propagating invalidation", file.prehash.data());
			invalidateFile(file.prehash);
			break;
		default:
			Log.e(_ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

void Peer::remoteFolderListener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	Index::origin_t origin;

	switch (status) {
		case Util::File::Status::created:
			break;
		case Util::File::Status::erased:
			deregisterFile(file.hash);
			exchanger->removeLocalFile(file);
			break;
		case Util::File::Status::modified:
			origin = indexer->getOrigin(file.prehash);
			origin.master = false;
			deregisterFile(file.prehash);
			exchanger->updateLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, origin);
			break;
		default:
			Log.e(_ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

void Peer::invalidationListener(Util::File file) {
	Index::origin_t origin = indexer->getOrigin(file.hash);
	if (origin.peerID != -1) {
		Index::conn_t conn = origin.conn;
		Log.d("Invalidator", "Origin server to connect: %s", conn.str().data());
		exchanger->download(Index::PeerResults(file.name, origin.peerID, conn), file.hash, false);
	} else {
		Log.e("Invalidator", "Origin server not found: %s", file.hash.data());
	}
}

void Peer::pullingListener(Index::Entry::searchEntry entry) {
	exchanger->download(Index::PeerResults(entry.name, entry.origin.peerID, entry.origin.conn), entry.hash, false);
}

void Peer::downloadListener(Util::File file, Index::origin_t origin) {
	origin.master = false;
	registerFile(file.path.filename().string(), file.hash, origin);
}

Index::origin_t Peer::originHandler(Index::entryHash_t hash) {
	Index::origin_t o = indexer->getOrigin(hash);
	o.master = false;
	return o;
}

Peer::Peer(uint32_t id, uint16_t listeningPort, std::string indexingIP, uint16_t indexingPort, std::string uploadPath, std::string downloadPath, bool pushing, bool pulling, time_t TTR) :id { id }, listeningPort { listeningPort }, indexingIP { indexingIP }, indexingPort { indexingPort }, uploadPath { uploadPath }, downloadPath { downloadPath }, pushing { pushing }, pulling { pulling }, _TTR {TTR} {
	_console.setPrompt("Client-" + std::to_string(id));
	_console.addParser(indexRPCFunc);
}
/*--------- end change ----------*/
Peer::~Peer() {
	stop();
}

void Peer::console() {
	if (originWatcher == nullptr || remoteWatcher == nullptr) {
		start();
	}
	_console.run(indexer, exchanger);
}

void Peer::start() {
	if (originWatcher != nullptr || remoteWatcher != nullptr) {
		Log.w(_ID, "Already started");
		return;
	}
/*--------- start change ----------*/
	indexer = new Index::Indexer(id, listeningPort, indexingIP, indexingPort, pushing, pulling, [&](Index::Entry::searchEntry entry) {pullingListener(entry); });
	indexer->start(_TTR);
	exchanger = new Exchanger::Exchanger(id, listeningPort, downloadPath, [&](Util::File file, Index::origin_t origin) {downloadListener(file, origin); }, [&](Util::File file) {invalidationListener(file); }, [&](Index::entryHash_t hash) {return originHandler(hash); }, [&](Index::entryHash_t hash, time_t TTR) { indexer->updateTTR(hash, TTR); });
	exchanger->setDefaultTTR(_TTR);
	originWatcher = new Util::Folder(uploadPath, [&](Util::File file, Util::File::Status status) {originFolderListener(file, status); });
	remoteWatcher = new Util::Folder(downloadPath, [&](Util::File file, Util::File::Status status) {remoteFolderListener(file, status); });
}

void Peer::stop() {
	if (originWatcher != nullptr)
		delete originWatcher;
	if (remoteWatcher != nullptr)
		delete remoteWatcher;
	if (exchanger != nullptr)
		delete exchanger;
	if (indexer != nullptr) {
		indexer->stop();
		delete indexer;
	}
	indexer = nullptr;
	exchanger = nullptr;
	remoteWatcher = nullptr;
}
/*--------- end change ----------*/