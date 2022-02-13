// Exchanger.h : Header file for your target.

#pragma once

#include <stdint.h>
#include <string>

#include "index.h"

namespace Exchanger {
	using std::string;
	using Index::entryHash_t;

	void start(uint32_t id, uint16_t listeningPort);

	void connect(Index::conn_t conn, entryHash_t hash);

	void connect(string ip, uint16_t port, entryHash_t hash);
}
