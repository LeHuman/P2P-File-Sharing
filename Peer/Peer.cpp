﻿#include <string>

#include "Folder.h"
#include "indexRPC.h"
#include "Log.h"
#include "Console.h"
#include "Exchanger.h"

using std::string;

const string ID = "Peer";

Index::Indexer *peerIndexer;
Exchanger::Exchanger *peerExchanger;

void registerFile(Index::Indexer &indexer, string fileName, Index::entryHash_t hash) {
	bool registered = indexer.registry(fileName, hash);
	if (registered) {
		Log.i(ID, "Registered hash: %s", fileName.data());
	} else {
		Log.e(ID, "Unable to registered hash: %s", fileName.data());
	}
}

void deregisterFile(Index::Indexer &indexer, Index::entryHash_t hash) {
	bool deregistered = indexer.deregister(hash);
	if (deregistered) {
		Log.i(ID, "Deregisted hash: %s", hash.data());
	} else {
		Log.e(ID, "Unable to deregister hash: %s", hash.data());
	}
}

void listener(Util::File file, Util::File::Status status) {
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	switch (status) {
		case Util::File::Status::created:
			registerFile(*peerIndexer, file.path.filename().string(), file.hash);
			peerExchanger->addLocalFile(file);
			break;
		case Util::File::Status::erased:
			peerExchanger->removeLocalFile(file);
			deregisterFile(*peerIndexer, file.hash);
			break;
		case Util::File::Status::modified:
			deregisterFile(*peerIndexer, file.prehash);
			peerExchanger->updateLocalFile(file);
			registerFile(*peerIndexer, file.path.filename().string(), file.hash);
			break;
		default:
			Log.e(ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

int main() {
	srand((unsigned int)time(NULL));

	int id = rand();

	Index::Indexer c(id, "localhost", 55555);
	Exchanger::Exchanger e(id, 55555);

	peerIndexer = &c;
	peerExchanger = &e;

	c.start();

	Util::watchFolder("../../../../testFolder", 1000, listener);

	Console::run(c, e);

	return 0;
}
