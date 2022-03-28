/**
 * @file LocalTesting.cpp
 * @author IR
 * @brief Program used only for testing
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <thread>
#include <string>
#include <time.h>
#include <random>

#include <rpc/server.h>
#include <rpc/rpc_error.h>

#include "Log.h"
#include "Peer.h"
#include "index.h"
#include "Console.h"
#include "Parsers.h"
#include "indexRPC.h"
#include "Exchanger.h"

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

	//Index::Indexer indexer(rndN(rng), 1337, ip, port);
	//indexer.start();

	//for (size_t i = 0; i < 16; i++) {
	//	switch (rndBool(rng)) {
	//		case 0:
	//			threads.push_back(new thread([&](string entryName, Index::entryHash_t hash) {indexer.registry(entryName, hash); }, rnd_names[rndN(rng)], std::to_string(rndDist(rng))));
	//			break;
	//		case 1:
	//			threads.emplace_back(new thread([&](Index::entryHash_t hash) {indexer.deregister(hash); }, std::to_string(rndDist(rng))));
	//			break;
	//		case 2:
	//			threads.emplace_back(new thread([&](Index::entryHash_t hash) {indexer.request(hash); }, std::to_string(rndDist(rng))));
	//			break;
	//		case 3:
	//			//threads.emplace_back(new thread([&](string query) {data->logSearch(query); }, "z"));
	//			//threads.emplace_back(new thread([&](string query) {data->logSearch(query); }, "am"));
	//			break;
	//	}
	//}

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

int main() {
	//Index::Indexer s(55555);
	//s.start();

	//./Client -i 1211 -c 44563 -s "192.168.1.200" -e 55555 -f "../../../../testFolder2"
	//./Client -i 156 -c 42910 -s "192.168.1.200" -e 55555 -f "../../../../testFolder"
	//./Client -i 791 -c 44910 -s "192.168.1.200" -e 55555 -f "../../../../testFolder"

	//Peer c(321, 46873, "192.168.1.231", 55555, "../../../../testFolder");
	//Peer c2(123, 37864, "192.168.1.231", 55555, "../../../../testFolder2");

	//c.start();
	//c2.start();

}
