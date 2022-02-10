// P2P File Sharing.cpp : Defines the entry point for the application.

#include <thread>
#include <string>
#include <time.h>
#include <random>

#include "rpc/server.h"

#include "Log.h"
#include "index.h"
#include "indexRPC.h"

using std::vector;
using std::string;
using std::pair;
using std::thread;

using Index::request;
using Index::registry;
using Index::deregister;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> rndDist(1024, 1028);

string rnd_names[] = {
	"namer",
	"whah",
	"amogus",
	"yaboi",
	"hijabi",
	"fugna123",
	"789456123",
};

Index::conn_t connections[] = {
	Index::conn_t("1.56.11.5", 165),
	Index::conn_t("1.5615.1.5.1",453),
	Index::conn_t("1.65.16.51.6.",231),
	Index::conn_t("416.5.1.65",6784),
	Index::conn_t("154.641.6.5",4532),
	Index::conn_t("416.1465.146.53",210),
	Index::conn_t("192.168.1.42",490),
};

std::uniform_int_distribution<std::mt19937::result_type> rndN(0, 6);
std::uniform_int_distribution<std::mt19937::result_type> rndBool(0, 3);

void threadedTest() {
	vector<thread *> threads;

	for (size_t i = 0; i < 16; i++) {
		switch (rndBool(rng)) {
			case 0:
				threads.push_back(new thread(registry, rndN(rng), connections[rndN(rng)], rnd_names[rndN(rng)], std::to_string(rndDist(rng))));
				break;
			case 1:
				threads.emplace_back(new thread(deregister, rndN(rng), connections[rndN(rng)], std::to_string(rndDist(rng))));
				break;
			case 2:
				threads.emplace_back(new thread(request, std::to_string(rndDist(rng))));
				break;
			case 3:
				threads.emplace_back(new thread(Index::logSearch, "z"));
				threads.emplace_back(new thread(Index::logSearch, "am"));
				break;
		}
	}

	for (thread *thr : threads) {
		thr->join();
		delete thr;
	}
}

void tieredThreadTest() {
	for (size_t i = 0; i < 500; i++) {
		thread a(threadedTest);
		thread b(threadedTest);
		thread c(threadedTest);
		thread d(threadedTest);
		a.join();
		b.join();
		c.join();
		d.join();
	}
}

#include "Console.h"

int main() {
	//tieredThreadTest();
	threadedTest();
	threadedTest();
	threadedTest();
	threadedTest();

	RPC::Indexer s(55555);
	RPC::Indexer c(456978, "localhost", 55555);

	s.start();
	c.start();

	Console::run(c);
}
