#include "Parsers.h"
#include "Folder.h"
#include "Peer.h"

using std::string;

static const string _ID = "Peer";

void Peer::registerFile(Index::Indexer &indexer, string fileName, Index::entryHash_t hash) {
	bool registered = indexer.registry(fileName, hash);
	if (registered) {
		Log.i(_ID, "Registered hash: %s", fileName.data());
	} else {
		Log.e(_ID, "Unable to registered hash: %s", fileName.data());
	}
}

void Peer::deregisterFile(Index::Indexer &indexer, Index::entryHash_t hash) {
	bool deregistered = indexer.deregister(hash);
	if (deregistered) {
		Log.i(_ID, "Deregistered hash: %s", hash.data());
	} else {
		Log.e(_ID, "Unable to deregister hash: %s", hash.data());
	}
}

void Peer::listener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	switch (status) {
		case Util::File::Status::created:
			exchanger->addLocalFile(file);
			registerFile(*indexer, file.path.filename().string(), file.hash);
			break;
		case Util::File::Status::erased:
			exchanger->removeLocalFile(file);
			deregisterFile(*indexer, file.hash);
			break;
		case Util::File::Status::modified:
			deregisterFile(*indexer, file.prehash);
			exchanger->updateLocalFile(file);
			registerFile(*indexer, file.path.filename().string(), file.hash);
			break;
		default:
			Log.e(_ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

Peer::Peer(uint32_t id, uint16_t listeningPort, std::string indexingIP, uint16_t indexingPort, std::string downloadPath) :id { id }, listeningPort { listeningPort }, indexingIP { indexingIP }, indexingPort { indexingPort }, downloadPath { downloadPath }{
	_console.setPrompt("Client");
	_console.addParser(indexRPCFunc);
}

Peer::~Peer() {
	stop();
}

void Peer::console() {
	if (folderWatcher == nullptr) {
		start();
	}
	_console.run(indexer, exchanger);
}

void Peer::start() {
	if (folderWatcher != nullptr) {
		Log.w(_ID, "Already started");
		return;
	}

	indexer = new Index::Indexer(id, listeningPort, indexingIP, indexingPort);
	indexer->start();
	exchanger = new Exchanger::Exchanger(id, listeningPort, downloadPath);
	folderWatcher = new Util::Folder(downloadPath, [&](Util::File file, Util::File::Status status) {listener(file, status); });
}

void Peer::stop() {
	if (folderWatcher != nullptr)
		delete folderWatcher;
	if (exchanger != nullptr)
		delete exchanger;
	if (indexer != nullptr) {
		indexer->stop();
		delete indexer;
	}
	indexer = nullptr;
	exchanger = nullptr;
	folderWatcher = nullptr;
}
