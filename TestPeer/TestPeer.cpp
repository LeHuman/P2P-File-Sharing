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

uint32_t id;
Index::Indexer *_indexer;
Exchanger::Exchanger *_exchanger;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> rndStr(0, testStrs.size() - 1);
std::uniform_int_distribution<std::mt19937::result_type> rndID(10000, 20000);
std::uniform_int_distribution<std::mt19937::result_type> rndBool(0, 1);
std::uniform_int_distribution<std::mt19937::result_type> rndDel(0, 5);
std::uniform_int_distribution<std::mt19937::result_type> rndEdit(0, 20);
std::uniform_int_distribution<std::mt19937::result_type> rndWait(200, 1000);

void registerFile(string fileName, Index::entryHash_t hash, Index::origin_t origin) {
	bool registered = _indexer->registry(fileName, hash, origin); // TODO: version number
	if (registered) {
		Log.i(ID, "Registered hash: %s", fileName.data());
	} else {
		Log.e(ID, "Unable to register hash: %s", fileName.data());
	}
}

void deregisterFile(Index::entryHash_t hash, bool master) {
	bool deregistered = _indexer->deregister(hash, master);
	if (deregistered) {
		Log.i(ID, "Deregistered hash: %s", hash.data());
	} else {
		Log.e(ID, "Unable to deregister hash: %s", hash.data());
	}
}

void invalidateFile(Index::entryHash_t hash) {
	bool invalidated = _indexer->invalidate(hash);
	if (invalidated) {
		Log.i(ID, "Invalidated hash: %s", hash.data());
	} else {
		Log.w(ID, "Unable to invalidate hash or no peers to locally invalidate: %s", hash.data());
	}
}

void originFolderListener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	switch (status) {
		case Util::File::Status::created:
			_exchanger->addLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, Index::origin_t(id, _indexer->getPeerConn(), true)); // TODO: run with master bit
			break;
		case Util::File::Status::erased:
			deregisterFile(file.hash, true);
			_exchanger->removeLocalFile(file);
			break;
		case Util::File::Status::modified:
			if (file.prehash == file.hash) {
				Log.w(ID, "File modified retained hash, not updating: %s", file.hash.data());
				break;
			}
			deregisterFile(file.prehash, true);
			_exchanger->updateLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, Index::origin_t(id, _indexer->getPeerConn(), true));
			Log.d("originFolderListener", "Propagating invalidation", file.prehash.data());
			invalidateFile(file.prehash);
			break;
		default:
			Log.e(ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

void remoteFolderListener(Util::File file, Util::File::Status status) { // TODO: not thread safe, peers can potentially connect with outdated info between calls
	if (!std::filesystem::is_regular_file(std::filesystem::path(file.path)) && status != Util::File::Status::erased) {
		return;
	}

	Index::origin_t origin;

	switch (status) {
		case Util::File::Status::created:
			break;
		case Util::File::Status::erased:
			deregisterFile(file.hash, false);
			_exchanger->removeLocalFile(file);
			break;
		case Util::File::Status::modified:
			origin = _indexer->getOrigin(file.prehash);
			origin.master = false;
			deregisterFile(file.prehash, false);
			_exchanger->updateLocalFile(file);
			registerFile(file.path.filename().string(), file.hash, origin);
			break;
		default:
			Log.e(ID, "Unknown file status: %s", file.path.filename().string().data());
	}
}

void invalidationListener(Util::File file) {
	Index::origin_t origin = _indexer->getOrigin(file.hash);
	if (origin.peerID != -1) {
		Index::conn_t conn = origin.conn;
		Log.d("Invalidator", "Origin server to connect: %s", conn.str().data());
		_exchanger->download(Index::PeerResults(file.name, origin.peerID, conn), file.hash, false);
	} else {
		Log.e("Invalidator", "Origin server not found: %s", file.hash.data());
	}
}

void pullingListener(Index::Entry::searchEntry entry) {
	_exchanger->download(Index::PeerResults(entry.name, entry.origin.peerID, entry.origin.conn), entry.hash, false);
}

void downloadListener(Util::File file, Index::origin_t origin) {
	origin.master = false;
	registerFile(file.path.filename().string(), file.hash, origin);
}

Index::origin_t originHandler(Index::entryHash_t hash) {
	Index::origin_t o = _indexer->getOrigin(hash);
	o.master = false;
	return o;
}

void outputText(string msg, string path) {
	std::ofstream ofs(path, std::ios_base::binary);
	for (int i = 0; i < (100 + (rndBool(rng) * 100)); i++) {
		ofs << msg.data();
	}
}

void requestFile(Index::entryHash_t hash) {
	Log.i(ID, "Searching server peer index for: %s", hash.data());
	Index::PeerResults results = _indexer->request(hash);
	_exchanger->download(results, hash);
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

bool editRndFile(string dir) {
	while (true) {
		for (const auto &entry : std::filesystem::directory_iterator(dir))
			if (!rndDel(rng)) {
				try {
					std::string app = testStrs[rndStr(rng)].substr(7);
					std::ofstream file(entry.path(), std::ios_base::app | std::ios_base::binary);
					file.write(app.data(), app.size());
				} catch (const std::exception &) {
					continue;
				}
				return true;
			}
	}
}

std::pair<Index::entryHash_t, float> getRandomListing() {
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
		return std::pair<Index::entryHash_t, float>("", 0);
	}

	Log.i(ID, "Get hash for: %s", out[0].name.data());
	return std::pair<Index::entryHash_t, float>(out[0].hash, out[0].icount);
}

int main(int argc, char *argv[]) {
	TCLAP::CmdLine cmd("Create a test P2P Client", ' ');

	TCLAP::ValueArg<uint32_t> idArg("i", "identity", "Unique ID identifying this client", true, 0, "int", cmd);
	TCLAP::ValueArg<std::string> confArg("c", "configFile", "The config file to use", false, "test_config.json", "filePath", cmd);
	TCLAP::ValueArg<std::string> upFldrArg("u", "uploadFolder", "The local folder files should be uploaded from", false, "", "directory", cmd);
	TCLAP::ValueArg<std::string> dnFldrArg("d", "downloadFolder", "The local folder files should be downloaded to", false, "", "directory", cmd);
	TCLAP::SwitchArg all2all("a", "all2all", "enable all2all mode", cmd);
	TCLAP::SwitchArg enabled("e", "enabled", "Whether this client should actively make queries", cmd);
	TCLAP::ValueArg<time_t> ttrArg("r", "ttr", "Time To Refresh (TTR) in seconds", false, 5, "int", cmd);
	TCLAP::SwitchArg pushingArg("s", "pushing", "enable pushing of invalidation calls", cmd);
	TCLAP::SwitchArg pullingArg("l", "pulling", "enable pulling of invalidation calls", cmd);
	cmd.parse(argc, argv);

	bool pushing = pushingArg.getValue();
	bool pulling = pullingArg.getValue();

	id = idArg.getValue();

	Config::config_t config = Config::getConfig(idArg.getValue(), confArg.getValue(), all2all.getValue());

	string serverIP = config.server.ip;
	uint16_t serverPort = config.server.port;
	uint16_t clientPort = config.port + 1000;

	string originFolder = upFldrArg.getValue();

	if (originFolder == "") {
		originFolder = "originFolder" + std::to_string(idArg.getValue());
	}

	if (!std::filesystem::exists(originFolder)) {
		std::filesystem::create_directories(originFolder);
		Log.w("Client", "Watch Folder auto created");
	}

	string remoteFolder = dnFldrArg.getValue();

	if (remoteFolder == "") {
		remoteFolder = "remoteFolder" + std::to_string(idArg.getValue());
	}

	if (!std::filesystem::exists(remoteFolder)) {
		std::filesystem::create_directories(remoteFolder);
		Log.w("Client", "Watch Folder auto created");
	}

	std::filesystem::path of(originFolder);

	for (const auto &entry : std::filesystem::directory_iterator(remoteFolder))
		std::filesystem::remove_all(entry.path());

	for (size_t i = 0; i < 10; i++) {
		outputText(std::to_string(id) + testStrs[rndStr(rng)], (of / ("testFile" + std::to_string(id) + "_" + std::to_string(i) + ".txt")).string());
	}
	
	Index::Indexer *s;

	Index::Indexer indexer(id, clientPort, serverIP, serverPort, pushing, pulling, [&](Index::Entry::searchEntry entry) {pullingListener(entry); });
	Exchanger::Exchanger exchanger(idArg.getValue(), clientPort, remoteFolder, [&](Util::File file, Index::origin_t origin) {downloadListener(file, origin); }, [&](Util::File file) {invalidationListener(file); }, [&](Index::entryHash_t hash) {return originHandler(hash); }, [&](Index::entryHash_t hash, time_t TTR) { _indexer->updateTTR(hash, TTR); });
	exchanger.setDefaultTTR(ttrArg.getValue());

	_indexer = &indexer;
	_exchanger = &exchanger;

	if (config.isSuper) { // Create server if this is a super peer
		s = new Index::Indexer(config.server.port, config.totalSupers, pushing, pulling, config.all2all);
		for (Index::conn_t conn : config.neighbors) {
			if (conn != config.server) { // Ensure we don't reference ourselves
				s->addNeighboor(conn);
			}
		}
		s->start();
	}

	std::filesystem::create_directories("csvs");
	std::ofstream csv((std::filesystem::path("csvs") / "mstime").string() + std::to_string(id) + ".csv", std::ios_base::binary);

	indexer.start(ttrArg.getValue());

	Util::Folder originWatcher(originFolder, [&](Util::File file, Util::File::Status status) {originFolderListener(file, status); });
	Util::Folder remoteWatcher (remoteFolder, [&](Util::File file, Util::File::Status status) {remoteFolderListener(file, status); });

	int c = 50;

	while (!std::filesystem::exists("start")) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if (enabled.getValue()) {
		Log.i("Test", "Enabled");

		try {
			while (indexer.connected() && c > 0) {
				float icount = 0;

				std::cout << c << std::endl;
				std::pair<Index::entryHash_t, float> lst = getRandomListing();
				Index::entryHash_t h = lst.first;

				std::this_thread::sleep_for(std::chrono::milliseconds(rndWait(rng)));

				if (h != "") {
					requestFile(h);
					icount = lst.second;
					c--;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				auto dirIter = std::filesystem::directory_iterator(remoteFolder);
				int fileCount = std::count_if(begin(dirIter), end(dirIter), [](auto &entry) { return entry.is_regular_file(); });

				if (fileCount > 0 && !rndDel(rng)) {
					deleteRndFile(remoteFolder);
				}

				csv << std::to_string(icount) << ',';
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		} catch (const std::exception &e) {
			Log.e("Test Error", "%s", e.what());
		}

		csv.flush();
		csv.close();

		std::ofstream d(std::to_string(id) + ".done");
		d.close();

	} else {
		Log.i("Standby", "Enabled");

		std::ofstream d(std::to_string(id) + ".done");
		d.close();

		try {
			while (!std::filesystem::exists("finish")) {
				std::cout << c << std::endl;

				std::this_thread::sleep_for(std::chrono::milliseconds(rndWait(rng)));

				if (!rndEdit(rng)) {
					editRndFile(originFolder);
				}
			}
		} catch (const std::exception &e) {
			Log.e("Test Error", "%s", e.what());
		}
	}

	std::cout << std::endl << "DONE" << std::endl;

	while (!std::filesystem::exists("finish")) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
