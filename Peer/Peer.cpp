#include <string>

#include "Folder.h"
#include "indexRPC.h"
#include "Log.h"
#include "Console.h"

using std::string;

const string ID = "Peer";

void registerFile(RPC::Indexer indexer, string fileName, Index::entryHash_t hash) {
	bool registered = indexer.registry(fileName, hash);
	if (registered) {
		Log.i(ID, "Registered hash: %s", fileName.data());
	} else {
		Log.e(ID, "Unable to registered hash: %s", fileName.data());
	}
}

void deregisterFile(RPC::Indexer indexer, Index::entryHash_t hash) {
	bool deregistered = indexer.deregister(hash);
	if (deregistered) {
		Log.i(ID, "Deregisted hash: %s", hash.data());
	} else {
		Log.e(ID, "Unable to deregister hash: %s", hash.data());
	}
}

void listener(RPC::Indexer indexer, Util::File file, Util::File::Status status) {
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path))) {
		return;
	}

	switch (status) {
		case Util::File::Status::created:
			registerFile(indexer, file.path.filename().string(), file.hash);
			break;
		case Util::File::Status::erased:
			deregisterFile(indexer, file.hash);
			break;
		case Util::File::Status::modified:
			deregisterFile(indexer, file.prehash);
			registerFile(indexer, file.path.filename().string(), file.hash);
			break;
		default:
			Log.e(ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

int main() {
	srand((unsigned int)time(NULL));

	RPC::Indexer c = RPC::Indexer(rand(), "localhost", 55555);
	Util::Folder fileFolder = Util::watchFolder("testFolder", 5000, std::bind(listener, c, std::placeholders::_1, std::placeholders::_2));

	c.start();

	Console::run(c);

	return 0;
}
