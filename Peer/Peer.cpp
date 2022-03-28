﻿/**
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
			invalidateFile(file.prehash);
			deregisterFile(file.prehash, true);
			exchanger->updateLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, Index::origin_t(id, indexer->getPeerConn(), true));
			break;
		default:
			Log.e(_ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

void Peer::remoteFolderListener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	Index::origin_t origin = indexer->getOrigin(file.prehash);
	origin.master = false;

	switch (status) { // TODO: only update exchanger if successfully registered
		case Util::File::Status::created:
			//exchanger->addLocalFile(file);
			//registerFile(file.path.filename().string(), file.hash, origin);
			break;
		case Util::File::Status::erased:
			deregisterFile(file.hash);
			exchanger->removeLocalFile(file);
			break;
		case Util::File::Status::modified:
			deregisterFile(file.prehash);
			exchanger->updateLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, origin);
			//Log.e(_ID, "Remote files cannot be modified, removing: %s", file.path.filename().string().data());
			// TODO: remove file
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
		exchanger->download(Index::PeerResults(file.name, origin.peerID, conn), file.name, false);
	} else {
		Log.e("Invalidator", "Origin server not found: %s", file.hash.data());
	}
}

void Peer::downloadListener(Util::File file, Index::origin_t origin) {
	origin.master = false;
	registerFile(file.path.filename().string(), file.hash, origin);
}

Peer::Peer(uint32_t id, uint16_t listeningPort, std::string indexingIP, uint16_t indexingPort, std::string uploadPath, std::string downloadPath, bool pushing, bool pulling) :id { id }, listeningPort { listeningPort }, indexingIP { indexingIP }, indexingPort { indexingPort }, uploadPath { uploadPath }, downloadPath { downloadPath }, pushing { pushing }, pulling { pulling } {
	_console.setPrompt("Client-" + std::to_string(id));
	_console.addParser(indexRPCFunc);
}

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

	indexer = new Index::Indexer(id, listeningPort, indexingIP, indexingPort, pushing, pulling);
	indexer->start();
	exchanger = new Exchanger::Exchanger(id, listeningPort, downloadPath, [&](Util::File file, Index::origin_t origin) {downloadListener(file, origin); }, [&](Util::File file) {invalidationListener(file); }, [&](Util::File file) {auto o = indexer->getOrigin(file.hash); o.master = false; return o; });
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
