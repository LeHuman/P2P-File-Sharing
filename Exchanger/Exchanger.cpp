/**
 * @file Exchanger.cpp
 * @author IR
 * @brief The source code for the exchanger module
 * @version 0.1
 * @date 2022-02-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <cstdlib>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <filesystem>

#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/connect.hpp>
#include <asio/signal_set.hpp>
#include <asio/basic_socket_iostream.hpp>

#include "Exchanger.h"
#include "Log.h"

namespace Exchanger {
	using std::thread;
	using asio::ip::tcp;

	constexpr std::size_t block_size = 1024 * 4;

	enum class State {
		C_Connect,
		S_Accept,
		C_ConfirmID,
		S_FindFile,
		C_AllocSpace,
		C_Retry,
		Streaming,
		Disconnect,
	};

	static const string ID = "Exchanger";
	static const string iID = "Exchanger Invalidation";
	static const string sID = "Exchanger Server";
	static const string cID = "Exchanger Client";

	void timeout(tcp::iostream &stream) {
		stream.expires_after(std::chrono::seconds(5));
	}

	void fileInvalidate(const Index::Peer &peer, entryHash_t hash) {
		try {
			Log.d(iID, "Connecting to peer: %d", peer.id);
			tcp::iostream stream(peer.connInfo.ip, std::to_string(peer.connInfo.port));
			stream.expires_after(std::chrono::seconds(60));
			Log.d(iID, "Sending mode");
			char mode = Mode::InvalidateFile;
			stream.write(&mode, 1);
			Log.d(iID, "sending hash: %s", hash.data());
			uint16_t hashLen = hash.size();

			stream.write((char *)&hashLen, sizeof(uint16_t));
			stream.write(hash.data(), hashLen);
		} catch (std::exception &e) {
			Log.e(sID, "Socket Server Exception: %s\n", e.what());
		}
	}

	/**
	 * @brief The state machine for the peer that is sending a file
	 */
	void Exchanger::fileSender(uint32_t id, tcp::iostream stream) {
		State state = State::C_Connect;
		uint16_t hashLen = 0;
		uint64_t fileSize = 0;
		uint64_t totalWritten = 0;
		double lastWritten = 0;
		char status = 0;
		std::streamsize written = 1;

		Mode mode;
		Index::origin_t origin;
		uint16_t port = 0;
		time_t newTTR = -1;

		asio::error_code error;
		std::string nameSearch;
		Index::entryHash_t hash;
		Util::File file;
		std::ifstream fileOut;
		std::unordered_map<Index::entryHash_t, Util::File>::const_iterator it;

		char sBuf[block_size] = { 0 };

		int fileRetry = 5;

		try {
			while (state != State::Disconnect) {
				timeout(stream);
				switch (state) {
					case State::C_Connect:
						stream.read(&status, 1);
						switch (status) { // Get connection mode
							case Mode::P2PFileUpdate:
								mode = Mode::P2PFileUpdate;
								state = State::S_Accept;
								Log.d(sID, "P2PFileUpdate");
								break;
							case Mode::P2PFile:
								mode = Mode::P2PFile;
								state = State::S_Accept;
								Log.d(sID, "P2PFile");
								break;
							case Mode::InvalidateFile:
								mode = Mode::InvalidateFile;
								state = State::S_FindFile;
								Log.d(sID, "InvalidateFile");
								break;
							default:
								Log.e(sID, "Invalid connection mode: %d", status);
								state = State::Disconnect;
								break;
						}
						timeout(stream);
						break;
					case State::S_Accept:
						Log.d(sID, "Sending ID");
						stream.write((char *)&id, sizeof(uint32_t));
						state = State::S_FindFile;
						timeout(stream);
						break;
					case State::S_FindFile:

						stream.read((char *)&hashLen, sizeof(uint16_t));
						if (hashLen >= block_size) { // TODO: remove limit
							Log.e(sID, "Hash size too large: %d", hashLen);
							state = State::Disconnect;
							break;
						}
						Log.d(sID, "Hash size get: %i", hashLen);

						stream.read(sBuf, hashLen);
						hash = Index::entryHash_t(sBuf); // Get Hash from Client
						Log.d(sID, "Hash get: %s", hash.data());

						newTTR = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + _TTR; // Default TTR + current time

						if (mode == Mode::P2PFileUpdate) {
							stream.read((char *)&hashLen, sizeof(uint16_t));
							if (hashLen >= block_size) { // TODO: remove limit
								Log.e(sID, "Name size too large: %d", hashLen);
								state = State::Disconnect;
								break;
							}
							stream.read(sBuf, hashLen);
							nameSearch = std::string(sBuf); // Get name from Client

							Log.d(sID, "Name get: %s", nameSearch.data());

							std::unique_lock<std::mutex> lock(localMutex);
							status = (it = localFileNames.find(nameSearch)) != localFileNames.end();

							if (!status) {
								Log.w(sID, "File does not exist: %s", nameSearch.data());
								state = State::Disconnect;
								break;
							}

							if (it->second.hash == hash) { // File on client and origin server match
								Log.w(sID, "Hashes are consistent: %s", hash.data());
								stream.write((char *)&newTTR, sizeof(time_t));
								state = State::Disconnect;
								break;
							} else {
								newTTR = -1;
							}
						} else {
							std::unique_lock<std::mutex> lock(localMutex);
							status = (it = localFiles.find(hash)) != localFiles.end();
						}

						stream.write((char *)&newTTR, sizeof(time_t));

						timeout(stream);

						if (status) { // File exists, open file and send size of file
							file = it->second;
							file.refresh(); // FIXME: file size returns zero sometimes? must be running in the middle of a origin server modification?
							if (mode == Mode::InvalidateFile) {
								stream.close();
								Log.d(sID, "Running invalidation listener for hash: %s", hash.data());
								invalidationListener(file);
								state = State::Disconnect;
								break;
							}
							fileOut = std::ifstream(file.path, std::ios_base::in | std::ios_base::binary); // TODO: ensure file has not changed, confirm localFiles matches details
							fileSize = file.size;
							Log.d(sID, "File found, sending size: %d", fileSize);
							stream.write((char *)&fileSize, sizeof(uint64_t));

							origin = originHandler(file.hash);

							timeout(stream);

							id = origin.peerID;
							port = origin.conn.port;
							hashLen = origin.conn.ip.size();

							stream.write((char *)&id, sizeof(uint32_t)); // Send id of origin
							stream.write((char *)&port, sizeof(uint16_t)); // Send port of origin
							stream.write((char *)&hashLen, sizeof(uint16_t)); // Send length of origin ip string
							stream.write(origin.conn.ip.data(), hashLen); // Send origin ip string

							state = State::Streaming;
						} else { // File does not exist, disconnect
							fileSize = 0;
							stream.write((char *)&fileSize, sizeof(uint64_t));
							Log.w(sID, "File does not exist: %s", hash.data());
							state = State::Disconnect;
						}
						break;
					case State::Streaming:
						Log.d(sID, "Waiting to stream");
						stream.read(&status, 1);
						timeout(stream);
						if (status) { // Get confirmation from client
							Log.d(sID, "Streaming");
							while (totalWritten != fileSize && !fileOut.eof()) {
								if (stream.error()) {
									Log.e(sID, "Stream error: %s@%lld", hash.data(), totalWritten);
									break;
								}
								if (fileOut.bad()) {
									if (!fileRetry) {
										break;
									}
									Log.e(sID, "File error, retrying: %s@%lld", hash.data(), totalWritten);
									fileRetry--;
									std::this_thread::sleep_for(std::chrono::milliseconds(100));
									fileOut = std::ifstream(file.path, std::ios_base::in | std::ios_base::binary);
									fileOut.seekg(totalWritten);
									continue;
								}
								fileOut.read(sBuf, block_size);
								written = fileOut.gcount();
								totalWritten += written;
								if (((double)totalWritten / fileSize) - lastWritten > 0.01) {
									lastWritten = (double)totalWritten / fileSize;
									Log.d(sID, "Written %lld%%", (uint64_t)(lastWritten * 100));
								}
								stream.write(sBuf, written);
								if (written > 0)
									timeout(stream);
							}
							if (totalWritten != fileSize) {
								Log.e(sID, "Sent filesize does not match: %lld", totalWritten);
							}
							Log.i(sID, "Finished streaming");
						} else {
							Log.w(sID, "Client unable to stream");
						}
						state = State::Disconnect;
						break;
					default:
						Log.e(sID, "Unknown connection state");
						state = State::Disconnect;
						break;
				}

				error = stream.error();
				if (error == asio::error::eof)			// Connection closed cleanly by peer.
					break;
				else if (error)							// Some other error.
					throw asio::system_error(error);
			}
		} catch (std::exception &e) {
			Log.e(sID, "Socket Server Exception: %s\n", e.what());
			stream.close();
		}
	}

	/**
	 * @brief The state machine for the peer that is receiving a file
	 */
	int Exchanger::fileReceiver(tcp::iostream &stream, uint32_t eid, Index::entryHash_t hash, string downloadPath, bool usingHash) {
		State state = State::C_Connect;
		uint16_t hashLen = hash.size();
		uint64_t fileSize = 0;
		char status = true;
		std::streamsize received = 1;
		std::streamsize totalReceived = 0;

		asio::error_code error;
		std::ofstream fileIn;

		char sBuf[block_size] = { 0 };
		uint32_t id = 0;

		Mode mode;
		Index::origin_t origin;
		std::string fileName = std::filesystem::path(downloadPath).filename().string();
		Util::File file;
		uint16_t port = 0;
		time_t newTTR = -1;

		int fileRetry = 5;

		try {
			while (state != State::Disconnect) {
				timeout(stream);
				switch (state) {
					case State::C_Connect:
						Log.d(cID, "Sending mode");
						if (usingHash) {
							mode = Mode::P2PFile;
							status = Mode::P2PFile;
						} else {
							mode = Mode::P2PFileUpdate;
							status = Mode::P2PFileUpdate;
						}
						stream.write(&status, 1);
						state = State::C_ConfirmID;
						timeout(stream);
						break;
					case State::C_ConfirmID:
						Log.d(cID, "Receiving server ID");
						stream.read((char *)&id, sizeof(uint32_t));
						if (id != eid) {
							Log.w(cID, "Mismatched ID: %d != %d", eid, id);
							status = false;
							state = State::Disconnect;
						} else {
							Log.d(cID, "ID match, sending hash: %s", hash.data());
							timeout(stream);
							stream.write((char *)&hashLen, sizeof(uint16_t));
							stream.write(hash.data(), hash.size());

							if (mode == Mode::P2PFileUpdate) {
								hashLen = fileName.size() + 1;
								stream.write((char *)&hashLen, sizeof(uint16_t));
								stream.write(fileName.data(), hashLen + 1);
							}

							stream.read((char *)&newTTR, sizeof(time_t));

							if (mode == Mode::P2PFileUpdate && newTTR > 0) {
								Log.i(cID, "File consistent, new TTR: %lld", newTTR);
								TTRListener(hash, newTTR);
								state = State::Disconnect;
								break;
							}

							state = State::C_AllocSpace;
						}
						break;
					case State::C_AllocSpace:
						Log.d(cID, "Receiving file size and origin");
						stream.read((char *)&fileSize, sizeof(uint64_t)); // Get filesize
						stream.read((char *)&id, sizeof(uint32_t)); // Get id of origin
						stream.read((char *)&port, sizeof(uint16_t)); // Get port of origin
						stream.read((char *)&hashLen, sizeof(uint16_t)); // Get length of origin ip string
						if (hashLen >= block_size) { // TODO: remove limit
							Log.e(sID, "ip size too large: %d", hashLen);
							status = false;
							state = State::Disconnect;
							break;
						}
						timeout(stream);
						stream.read(sBuf, hashLen); // Get origin ip string

						origin = Index::origin_t(id, Index::conn_t(std::string(sBuf), port), false);

						timeout(stream);
						if (fileSize == 0) { // TODO: Check file size for mistmatch and also allocate file / check for space
							status = false;
							Log.w(cID, "Invalid filesize: %d", fileSize);
							stream.write(&status, 0);
							state = State::Disconnect;
						} else {
							fileIn = std::ofstream(downloadPath, std::ios_base::out | std::ios_base::binary);
							Log.d(cID, "Origin get: %s", origin.str().data());
							Log.d(cID, "Valid filesize, notifying server: %d", fileSize);
							status = true;
							stream.write(&status, 1);
							state = State::Streaming;
						}
						break;
					case State::Streaming:
						Log.d(cID, "Streaming");
						while (totalReceived != fileSize) {
							if (stream.error()) {
								Log.e(sID, "Stream error: %s@%lld", hash.data(), totalReceived);
								return 2;
							}
							if (fileIn.bad()) {
								if (!fileRetry) {
									break;
								}
								Log.e(sID, "File error, retrying: %s@%lld", hash.data(), totalReceived);
								fileRetry--;
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								fileIn = std::ofstream(downloadPath, std::ios_base::app | std::ios_base::binary);
								continue;
							}
							stream.read(sBuf, block_size);
							received = stream.gcount();
							fileIn.write(sBuf, received);
							totalReceived += received;
							if (received > 0)
								timeout(stream);
						}
						Log.i(cID, "Finished streaming, storing local file");
						stream.close();
						if (totalReceived != fileSize) {
							Log.e(cID, "Received filesize does not match: %lld", totalReceived);
						}
						fileIn.flush();
						file = Util::File(downloadPath);
						if (usingHash && file.hash != hash) {
							Log.e(cID, "Keys do not match, unable to index: %s", hash.data());
							status = false;
							state = State::Disconnect;
							break;
						}
						updateLocalFile(file);
						Log.d(cID, "Running finished listener for hash: %s", hash.data());
						downloadListener(file, origin);
						TTRListener(hash, newTTR);
						status = true;
						state = State::Disconnect;
						break;
					default:
						Log.e(cID, "Unknown connection state");
						status = false;
						state = State::Disconnect;
						break;
				}
				// TODO: check final hash matches
				error = stream.error();
				if (error == asio::error::eof)			// Connection closed cleanly by peer.
					break;
				else if (error)							// Some other error.
					throw asio::system_error(error);
			}
		} catch (std::exception &e) {
			Log.e(cID, "Socket Client Exception: %s\n", e.what());
			stream.close();
			return false;
		}
		return status;
	}

	/**
	 * @brief Threaded function that is constantly running, listening for peers to accept a socket connection to
	 */
	void Exchanger::listener(uint32_t id, uint16_t port) {
		try {
			tcp::acceptor acceptor(*io_context, tcp::endpoint(tcp::v4(), port));

			while (true) {
				tcp::iostream stream;
				stream.expires_after(std::chrono::seconds(5));
				acceptor.accept(stream.socket());

				auto endpnt = stream.socket().remote_endpoint();
				Log.d(ID, "p2p conn: %s:%d", endpnt.address().to_string().data(), endpnt.port());
				thread(&Exchanger::fileSender, this, id, std::move(stream)).detach();
			}
		} catch (const std::exception &e) {
			Log.e(ID, "Error listening for requests: %s", e.what());
		}
	}


	/**
	 * @brief Given a list of peers, this threaded function will attempt to connect to peers until it downloads a file
	 */
	void Exchanger::peerResolver(Index::PeerResults results, std::string key, string downloadPath, bool usingHash) { // TODO: smarter peer selection & chunked file downloading
		int retries = 3;

		for (std::vector<Index::Peer::searchEntry>::iterator mit = results.peers.begin(); mit != results.peers.end(); ) {
			Index::Peer::searchEntry &item = *mit;

			Log.d(ID, "Connecting to peer: %d:%s:%i", item.id, item.connInfo.ip.data(), item.connInfo.port);
			tcp::iostream stream(item.connInfo.ip, std::to_string(item.connInfo.port));
			stream.expires_after(std::chrono::seconds(60));
			int result = fileReceiver(stream, item.id, key, downloadPath, usingHash);

			if (result == 1) {
				return;
			} else if (!retries) {
				break;
			} else if (result == 2) {
				retries--;
				Log.w(ID, "Retrying to connect to peer: %d", item.id);
				continue;
			} else {
				mit++;
			}
		}

		Log.f(ID, "Failed to request file: %s", results.fileName.data());
	}

	/**
	 * @brief Threaded function that is constantly running, waiting for request to download a file
	 */
	void Exchanger::receiver() { // TODO: set timeout
		try {
			tcp::resolver resolver(*io_context);

			while (running) {
				std::unique_lock<std::mutex> lock(mutex);
				if (cond.wait_for(lock, std::chrono::milliseconds(50), [&]() {return !queries.empty(); })) {
					const struct query_t query = queries.front();
					queries.pop();

					thread(&Exchanger::peerResolver, this, std::move(query.results), query.key, query.downloadPath, query.usingHash).detach();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		} catch (const std::exception &e) {
			Log.e(ID, "Error receiving requests: %s", e.what());
		}

		Log.d(ID, "stopped receiving peer requests");
		running = true;
	}

	/**
	 * @brief Threaded function that runs relevant socket functions
	 */
	void Exchanger::_startSocket(uint32_t id, uint16_t listeningPort) {
		try {
			asio::signal_set signals(*io_context, SIGINT, SIGTERM);
			signals.async_wait([&](auto, auto) { io_context->stop(); });

			thread(&Exchanger::listener, this, id, listeningPort).detach();
			thread(&Exchanger::receiver, this).detach();

			io_context->run();
		} catch (std::exception &e) {
			Log.e(ID, "Socket Exception: %s\n", e.what());
		}
	}

	Exchanger::~Exchanger() {
		stop();
	}

	void Exchanger::stop() {
		if (io_context != nullptr)
			io_context->stop();
		running = false;
		while (!running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void Exchanger::setDownloadPath(string downloadPath) {
		this->downloadPath = downloadPath;
	}

	Exchanger::Exchanger(uint32_t id, uint16_t listeningPort, string downloadPath, const std::function<void(Util::File, Index::origin_t)> &downloadListener, const std::function<void(Util::File)> &invalidationListener, std::function<Index::origin_t(entryHash_t)> originHandler, std::function<void(Index::entryHash_t, time_t)> TTRListener) : downloadListener { downloadListener }, invalidationListener { invalidationListener }, originHandler { originHandler }, TTRListener { TTRListener } {
		setDownloadPath(downloadPath);
		io_context = new asio::io_context(thread::hardware_concurrency());
		thread(&Exchanger::_startSocket, this, id, listeningPort).detach();
		Log.d(ID, "Listening to port: %d", listeningPort);
	}

	bool Exchanger::download(Index::PeerResults results, std::string hash, bool usingHash) {
		std::filesystem::path filePath = downloadPath;
		filePath /= results.fileName;
		try {
			if (usingHash) {
				std::unique_lock<std::mutex> lock(localMutex);
				auto localFile = localFiles.find(hash);
				if (localFile != localFiles.end()) { // TODO: Check if file with same name exists
					Log.e(ID, "File already exists locally, not downloading: %s", localFile->second.name.data());
					return false;
				}
			} // else ignore if file already exists
		} catch (const Util::File::not_regular_error &) {
		}
		std::lock_guard<std::mutex> lock(mutex);
		queries.emplace(results, hash, filePath.string(), usingHash); // TODO: Allow per request download path
		cond.notify_all();
		return true;
	}

	void Exchanger::addLocalFile(Util::File file) {
		std::unique_lock<std::mutex> lock(localMutex);
		localFiles.emplace(file.hash, file);
		localFileNames.emplace(file.name, file); // TODO: make reference, not direct copy
	}

	void Exchanger::removeLocalFile(Util::File file) {
		std::unique_lock<std::mutex> lock(localMutex);
		localFiles.erase(file.hash);
		localFileNames.erase(file.name);
	}

	void Exchanger::updateLocalFile(Util::File file) { // TODO: mutex maps
		std::unique_lock<std::mutex> lock(localMutex);
		localFiles.erase(file.prehash);
		localFiles.emplace(file.hash, file).first->second = file;
		localFileNames.emplace(file.name, file).first->second = file;
	}

	void Exchanger::setDefaultTTR(time_t TTR) {
		_TTR = TTR;
	}
}
