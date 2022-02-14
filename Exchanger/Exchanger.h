// Exchanger.h : Header file for your target.

#pragma once

#include <stdint.h>
#include <string>

#include "index.h"
#include "Folder.h"

namespace Exchanger {
	using std::string;
	using Index::entryHash_t;

	void start(int id, uint16_t listeningPort);

	void connect(Index::conn_t conn, int id, entryHash_t hash, string filePath);

	void connect(int id, string ip, uint16_t port, entryHash_t hash, string filePath);

	void addLocalFile(Util::File file);

	void removeLocalFile(Util::File file);

	void updateLocalFile(Util::File file);
}
