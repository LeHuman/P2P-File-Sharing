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

void threadedTest(string ip, uint16_t port) {
	vector<thread *> threads;

	Index::Indexer indexer(rndN(rng),"NotAnActualAddress", 1337, ip, port);
	indexer.start();

	for (size_t i = 0; i < 16; i++) {
		switch (rndBool(rng)) {
			case 0:
				threads.push_back(new thread([&](string entryName, Index::entryHash_t hash) {indexer.registry(entryName, hash); }, rnd_names[rndN(rng)], std::to_string(rndDist(rng))));
				break;
			case 1:
				threads.emplace_back(new thread([&](Index::entryHash_t hash) {indexer.deregister(hash); }, std::to_string(rndDist(rng))));
				break;
			case 2:
				threads.emplace_back(new thread([&](Index::entryHash_t hash) {indexer.request(hash); }, std::to_string(rndDist(rng))));
				break;
			case 3:
				//threads.emplace_back(new thread([&](string query) {data->logSearch(query); }, "z"));
				//threads.emplace_back(new thread([&](string query) {data->logSearch(query); }, "am"));
				break;
		}
	}

	for (thread *thr : threads) {
		thr->join();
		delete thr;
	}
}

void tieredThreadTest(string ip, uint16_t port) {
	vector<thread *> threads;

	for (size_t i = 0; i < 100; i++) {
		threads.emplace_back(new thread(threadedTest, ip, port));
		threads.emplace_back(new thread(threadedTest, ip, port));
		threads.emplace_back(new thread(threadedTest, ip, port));
		threads.emplace_back(new thread(threadedTest, ip, port));
	}

	for (thread *thr : threads) {
		thr->join();
		delete thr;
	}
}

#include "Console.h"
#include "Exchanger.h"
#include <rpc/rpc_error.h>

int main() {
	Index::Indexer s(55555);
	Index::Indexer c(321, "localhost", 41567, "localhost", 55555);
	//Index::Indexer c2(123, "localhost", 55555);

	Exchanger::Exchanger e(321, 41567);
	//Exchanger::Exchanger e2(123, 46214);

	s.start();
	c.start();
	//c2.start();
	//s.stop();

	//tieredThreadTest("localhost", 55555);

	threadedTest("localhost", 55555);
	threadedTest("localhost", 55555);
	threadedTest("localhost", 55555);
	threadedTest("localhost", 55555);

	//Util::File file("../../../../testFolder2/file.bin");

	//e.addLocalFile(file);
	//e2.connect(321, "localhost", 41567, file.hash, "../../../../testFolder/file-copy.bin");

	//c.registry(file.name, file.hash);

	/*while (true) {
	}*/

	Console::run(c, e);
}
