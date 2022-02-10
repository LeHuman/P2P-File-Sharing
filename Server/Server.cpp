#include "indexRPC.h"
#include "Log.h"
#include "Console.h"

int main() {
	RPC::Indexer s = RPC::Indexer(55555);
	s.start();

	Console::run(s);

	return 0;
}
