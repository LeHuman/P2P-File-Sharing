#include <string>

#include "Folder.h"
#include "indexRPC.h"
#include "Log.h"
#include "Console.h"
#include "Exchanger.h"

using std::string;

const string ID = "Peer";

Index::Indexer *peerIndexer;

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
			Exchanger::addLocalFile(file);
			break;
		case Util::File::Status::erased:
			Exchanger::removeLocalFile(file);
			deregisterFile(*peerIndexer, file.hash);
			break;
		case Util::File::Status::modified:
			deregisterFile(*peerIndexer, file.prehash);
			Exchanger::updateLocalFile(file);
			registerFile(*peerIndexer, file.path.filename().string(), file.hash);
			break;
		default:
			Log.e(ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

int main() {
	srand((unsigned int)time(NULL));

	Index::Indexer c = Index::Indexer(rand(), "localhost", 55555);

	peerIndexer = &c;

	c.start();

	Util::watchFolder("../../../../testFolder", 1000, listener);

	Console::run(c);

	return 0;
}
