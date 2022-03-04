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
#include "TestStrings.h"
#include <Config.h>

using std::string;

static const string ID = "TestPeer";

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
		Log.i(ID, "Deregistered hash: %s", hash.data());
	} else {
		Log.e(ID, "Unable to deregister hash: %s", hash.data());
	}
}

Index::Indexer *_indexer;
Exchanger::Exchanger *_exchanger;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> rndStr(0, testStrs.size() - 1);
std::uniform_int_distribution<std::mt19937::result_type> rndID(0, 20000);
std::uniform_int_distribution<std::mt19937::result_type> rndBool(0, 1);
std::uniform_int_distribution<std::mt19937::result_type> rndDel(0, 5);

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

bool requestFile(Index::entryHash_t hash) {
	Log.i(ID, "Searching server peer index for: %s", hash.data());
	Index::PeerResults results = _indexer->request(hash);
	return _exchanger->download(results, hash);
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
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
	outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());

	Index::Indexer *s;

	Index::Indexer indexer(id, clientPort, serverIP, serverPort);
	Exchanger::Exchanger exchanger(idArg.getValue(), clientPort, folder);

	_indexer = &indexer;
	_exchanger = &exchanger;

	if (config.isSuper) { // Create server if this is a super peer
		s = new Index::Indexer(config.server.port, config.totalSupers, config.all2all);
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

	if (enabled) {
		try {
			while (indexer.connected() && c > 0) {
				std::chrono::microseconds duration;

				std::cout << c << std::endl;
				Index::entryHash_t h = getRandomListing();
				if (h != "") {
					auto start = std::chrono::high_resolution_clock::now();
					c -= requestFile(h);
					auto stop = std::chrono::high_resolution_clock::now();
					duration = duration_cast<std::chrono::microseconds>(stop - start);
				}

				auto dirIter = std::filesystem::directory_iterator(folder);
				int fileCount = std::count_if(begin(dirIter), end(dirIter), [](auto &entry) { return entry.is_regular_file(); });

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				if (fileCount > 5 && rndDel(rng)) {
					deleteRndFile(folder);
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				if (fileCount > 5 && rndDel(rng)) {
					outputText(testStrs[rndStr(rng)], (fp / ("testFile" + std::to_string(rndID(rng)) + ".txt")).string());
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				//auto ping = indexer.ping().count();
				//c--;
				csv << duration << ',';
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		} catch (const std::exception &) {
		}
	}

	csv.flush();
	std::cout << std::endl << "DONE" << std::endl;
	csv.close();

	std::ofstream d(std::to_string(id) + ".done");
	d.close();

	while (!std::filesystem::exists("finish")) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
