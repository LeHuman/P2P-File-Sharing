#include "indexRPC.h"
#include "Log.h"
#include "Console.h"

int main(int argc, char *argv[]) {
	Index::Indexer s(55555);
	s.start();

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
