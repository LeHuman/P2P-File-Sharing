/**
 * @file TestPeer.cpp
 * @author IR
 * @brief Automated Peer, used only for testing
 * @version 0.2
 * @date 2022-03-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <filesystem>

#include <tclap/CmdLine.h>

#include "Exchanger.h"
#include "indexRPC.h"
#include "index.h"
#include "TestStrings.h"
#include <Config.h>

using std::string;

static const string ID = "TestPeer";

void registerFile(Index::Indexer &indexer, string fileName, Index::entryHash_t hash) {
	//bool registered = indexer.registry(fileName, hash);
	//if (registered) {
	//	Log.i(ID, "Registered hash: %s", fileName.data());
	//} else {
	//	Log.e(ID, "Unable to registered hash: %s", fileName.data());
	//}
}

void deregisterFile(Index::Indexer &indexer, Index::entryHash_t hash) {
	//bool deregistered = indexer.deregister(hash);
	//if (deregistered) {
	//	Log.i(ID, "Deregistered hash: %s", hash.data());
	//} else {
	//	Log.e(ID, "Unable to deregister hash: %s", hash.data());
	//}
}

Index::Indexer *_indexer;
Exchanger::Exchanger *_exchanger;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> rndStr(0, testStrs.size() - 1);
std::uniform_int_distribution<std::mt19937::result_type> rndID(0, 20000);
std::uniform_int_distribution<std::mt19937::result_type> rndBool(0, 1);
std::uniform_int_distribution<std::mt19937::result_type> rndDel(0, 5);
std::uniform_int_distribution<std::mt19937::result_type> rndWait(200, 1000);

void listener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	switch (status) {
		case Util::File::Status::created:
			_exchanger->addLocalFile(file);
			registerFile(*_indexer, file.path.filename().string(), file.hash);
			break;
		case Util::File::Status::erased:
			_exchanger->removeLocalFile(file);
			deregisterFile(*_indexer, file.hash);
			break;
		case Util::File::Status::modified:
			deregisterFile(*_indexer, file.prehash);
			_exchanger->updateLocalFile(file);
			registerFile(*_indexer, file.path.filename().string(), file.hash);
			break;
		default:
			Log.e(ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

void outputText(string msg, string path) {
	std::ofstream ofs(path, std::ios_base::binary);
	for (int i = 0; i < (100 + (rndBool(rng) * 100)); i++) {
		ofs << msg.data();
	}
}

std::chrono::microseconds requestFile(Index::entryHash_t hash) {
	Log.i(ID, "Searching server peer index for: %s", hash.data());
	auto start = std::chrono::high_resolution_clock::now();
	Index::PeerResults results = _indexer->request(hash);
	auto stop = std::chrono::high_resolution_clock::now();
	_exchanger->download(results, hash);
	return duration_cast<std::chrono::microseconds>(stop - start);
}

bool deleteRndFile(string dir) {
	while (true) {
		for (const auto &entry : std::filesystem::directory_iterator(dir))
			if (!rndDel(rng)) {
				try {
					std::filesystem::remove(entry);
				} catch (const std::exception &) {
					continue;
				}
				return true;
			}
		if (std::filesystem::is_empty(dir))
			return false;
	}
}

Index::entryHash_t getRandomListing() {
	Log.i(ID, "Listing server file index");
	Index::EntryResults listings = _indexer->list();
	Index::EntryResults out;
	std::sample(
		listings.begin(),
		listings.end(),
		std::back_inserter(out),
		1,
		std::mt19937 { std::random_device{}() }
	);

	if (!out.size()) {
		return "";
	}

	Log.i(ID, "Get hash for: %s", out[0].name.data());
	return out[0].hash;
}

int main(int argc, char *argv[]) {
	TCLAP::CmdLine cmd("Create a test P2P Client", ' ');

	TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", true, 0, "int", cmd);
	TCLAP::ValueArg<std::string> confArg("c", "configFile", "The config file to use", false, "test_config.json", "filePath", cmd);
	TCLAP::SwitchArg all2all("a", "all2all", "enable all2all mode", cmd);
	TCLAP::SwitchArg enabled("e", "enabled", "Whether this client should actively make queries", cmd);
	cmd.parse(argc, argv);

	uint32_t id = idArg.getValue();

	Config::config_t config = Config::getConfig(idArg.getValue(), confArg.getValue(), all2all.getValue());

	string serverIP = config.server.ip;
	uint16_t serverPort = config.server.port;
	uint16_t clientPort = config.port + 1000;
	string folder = "watchFolder" + std::to_string(id);
	std::filesystem::create_directories(folder);
	std::filesystem::path fp(folder);

	outputText(std::to_string(id), (fp / ("testFile" + std::to_string(id) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	
	Index::Indexer *s;

	Index::Indexer indexer(id, clientPort, serverIP, serverPort, config.pushing, config.pulling);
	Exchanger::Exchanger exchanger(idArg.getValue(), clientPort, folder, [&](Util::File file, Index::origin_t origin) {}, [&](Util::File file) {}, [&](Util::File file) {return indexer.getOrigin(file.hash); }, [&](Index::entryHash_t, time_t TTR) {});

	_indexer = &indexer;
	_exchanger = &exchanger;

	if (config.isSuper) { // Create server if this is a super peer
		s = new Index::Indexer(config.server.port, config.totalSupers, config.pushing, config.pulling, config.all2all);
		for (Index::conn_t conn : config.neighbors) {
			if (conn != config.server) { // Ensure we don't reference ourselves
				s->addNeighboor(conn);
			}
		}
		s->start();
	}

	std::filesystem::create_directories("csvs");
	std::ofstream csv((std::filesystem::path("csvs") / "mstime").string() + std::to_string(id) + ".csv", std::ios_base::binary);

	indexer.start();

	Util::Folder folderWatcher(folder, [&](Util::File file, Util::File::Status status) {listener(file, status); });

	int c = 200;

	//Log.enable(false);

	while (!std::filesystem::exists("start")) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if (enabled.getValue()) {
		Log.i("Test", "Enabled");

		try {
			while (indexer.connected() && c > 0) {
				std::chrono::microseconds duration;

				std::cout << c << std::endl;
				Index::entryHash_t h = getRandomListing();

				std::this_thread::sleep_for(std::chrono::milliseconds(rndWait(rng)));

				if (h != "") {
					duration = requestFile(h);
					c--;
				}

				auto dirIter = std::filesystem::directory_iterator(folder);
				int fileCount = std::count_if(begin(dirIter), end(dirIter), [](auto &entry) { return entry.is_regular_file(); });

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (fileCount > 0 && rndDel(rng)) {
					deleteRndFile(folder);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (fileCount < 1 && rndDel(rng)) {
					outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
				}

				csv << duration.count() << ',';
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		} catch (const std::exception &e) {
			Log.e("Test Error", "%s", e.what());
		}
	}

	csv.flush();
	csv.close();
	std::ofstream d(std::to_string(id) + ".done");
	d.close();

	std::cout << std::endl << "DONE" << std::endl;

	while (!std::filesystem::exists("finish")) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
