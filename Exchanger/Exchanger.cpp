﻿/**
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
	static const string sID = "Exchanger Server";
	static const string cID = "Exchanger Client";

	void timeout(tcp::iostream &stream) {
		stream.expires_after(std::chrono::seconds(5));
	}

    /**
     * @brief The state machine for the peer that is sending a file
     */
	void Exchanger::fileSender(uint32_t id, tcp::iostream stream) {
		State state = State::S_Accept;
		uint16_t hashLen = 0;
		uint64_t fileSize = 0;
		uint64_t totalWritten = 0;
		double lastWritten = 0;
		char status = 0;
		std::streamsize written = 1;

		asio::error_code error;
		Index::entryHash_t hash;
		Util::File file;
		std::ifstream fileOut;
		std::unordered_map<Index::entryHash_t, Util::File>::const_iterator it;

		char sBuf[block_size] = { 0 };

		try {
			while (state != State::Disconnect) {
				timeout(stream);
				switch (state) {
					case State::S_Accept:
						Log.d(sID, "Sending ID");
						stream.write((char *)&id, sizeof(uint32_t));
						state = State::S_FindFile;
						break;
					case State::S_FindFile:
						Log.d(sID, "Reading hash size");
						stream.read((char *)&hashLen, sizeof(uint16_t));
						if (hashLen >= block_size) { // TODO: remove limit
							Log.e(sID, "Hash size too large: %d", hashLen);
							state = State::Disconnect;
							break;
						}
						timeout(stream);
						stream.read(sBuf, hashLen);
						hash = Index::entryHash_t(sBuf); // Get Hash from Client

						Log.d(sID, "Hash get: %s", hash.data());

						timeout(stream);
						if ((it = localFiles.find(hash)) != localFiles.end()) { // File exists, open file and send size of file
							file = it->second;
							fileOut = std::ifstream(file.path, std::ios_base::in | std::ios_base::binary); // TODO: ensure file has not changed, confirm localFiles matches details
							fileSize = file.size;
							Log.d(sID, "File found, sending size: %d", fileSize);
							stream.write((char *)&fileSize, sizeof(uint64_t));
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
							while (written > 0 && !fileOut.eof() && !fileOut.bad() && !stream.error()) {
								timeout(stream);
								fileOut.read(sBuf, block_size);
								written = fileOut.gcount();
								totalWritten += written;
								if (((double)totalWritten / fileSize) - lastWritten > 0.01) {
									lastWritten = (double)totalWritten / fileSize;
									Log.d(sID, "Written %lld%%", (uint64_t)(lastWritten * 100));
								}
								stream.write(sBuf, written);
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
	bool Exchanger::fileReceiver(tcp::iostream &stream, uint32_t eid, entryHash_t hash, string downloadPath) {
		State state = State::C_ConfirmID;
		uint16_t hashLen = hash.size();
		uint64_t fileSize = 0;
		char status = true;
		std::streamsize received = 1;

		asio::error_code error;
		std::ofstream fileIn;

		char sBuf[block_size] = { 0 };
		uint32_t id = 0;

		try {
			while (state != State::Disconnect) {
				timeout(stream);
				switch (state) {
					case State::C_ConfirmID:
						Log.d(cID, "Receiving server ID");
						stream.read((char *)&id, sizeof(uint32_t));
						if (id != eid) {
							Log.w(cID, "Mismatched ID: %d != %d", eid, id);
							status = false;
							state = State::Disconnect;
						} else {
							Log.d(cID, "ID match, sending hash: %s", hash.data());
							stream.expires_after(std::chrono::seconds(5));
							stream.write((char *)&hashLen, sizeof(uint16_t));
							stream.write(hash.data(), hash.size());
							state = State::C_AllocSpace;
						}
						break;
					case State::C_AllocSpace:
						Log.d(cID, "Receiving file size");
						stream.read((char *)&fileSize, sizeof(uint64_t));
						timeout(stream);
						if (fileSize == 0) { // TODO: Check file size for mistmatch and also allocate file / check for space
							status = false;
							Log.w(cID, "Invalid filesize: %d", fileSize);
							stream.write(&status, 0);
							state = State::Disconnect;
						} else {
							fileIn = std::ofstream(downloadPath, std::ios_base::out | std::ios_base::binary);
							Log.d(cID, "Valid filesize, notifying server: %d", fileSize);
							stream.write(&status, 1);
							state = State::Streaming;
						}
						break;
					case State::Streaming:
						Log.d(cID, "Streaming");
						while (received > 0 && !fileIn.bad() && !stream.error()) {
							timeout(stream);
							stream.read(sBuf, block_size);
							received = stream.gcount();
							fileIn.write(sBuf, received);
						}
						Log.i(cID, "Finished streaming");
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
		tcp::acceptor acceptor(*io_context, tcp::endpoint(tcp::v4(), port));

		while (true) {
			tcp::iostream stream;
			stream.expires_after(std::chrono::seconds(5));
			acceptor.accept(stream.socket());

			auto endpnt = stream.socket().remote_endpoint();
			Log.d(ID, "p2p conn: %s:%d", endpnt.address().to_string().data(), endpnt.port());
			thread(&Exchanger::fileSender, this, id, std::move(stream)).detach();
		}
	}


    /**
     * @brief Given a list of peers, this threaded function will attempt to connect to peers until it downloads a file
     */
	void Exchanger::peerResolver(Index::PeerResults results, Index::entryHash_t hash, string downloadPath) { // TODO: smarter peer selection & chunked file downloading
		for (Index::Peer::searchEntry &item : results.peers) {
			Log.d(ID, "Connecting to peer: %d", item.id);
			tcp::iostream stream(item.connInfo.ip, std::to_string(item.connInfo.port));
			stream.expires_after(std::chrono::seconds(60));
			if (fileReceiver(stream, item.id, hash, downloadPath))
				return;
		}
        Log.f(ID, "Failed to request file: %s", downloadPath.data());
	}

    /**
     * @brief Threaded function that is constantly running, waiting for request to download a file
     */
	void Exchanger::receiver() { // TODO: set timeout
		tcp::resolver resolver(*io_context);
		while (running) {
			std::unique_lock<std::mutex> lock(mutex);
			if (cond.wait_for(lock, std::chrono::milliseconds(50), [&]() {return !queries.empty(); })) {
				const struct query_t query = queries.front();
				queries.pop();

				thread(&Exchanger::peerResolver, this, std::move(query.results), query.hash, query.downloadPath).detach();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

	Exchanger::Exchanger(uint32_t id, uint16_t listeningPort, string downloadPath) {
		setDownloadPath(downloadPath);
		io_context = new asio::io_context(thread::hardware_concurrency());
		thread(&Exchanger::_startSocket, this, id, listeningPort).detach();
		Log.d(ID, "Listening to port: %d", listeningPort);
	}

	bool Exchanger::download(Index::PeerResults results, entryHash_t hash) {
		std::filesystem::path filePath = downloadPath;
		filePath /= results.fileName;
		try {
			auto localFile = localFiles.find(hash);
			if (localFile != localFiles.end()) { // TODO: Check if file with same name exists
				Log.e(ID, "File already exists locally, not downloading: %s", localFile->second.name.data());
				return false;
			}
		} catch (const Util::File::not_regular_error &) {
		}
		std::lock_guard<std::mutex> lock(mutex);
		queries.emplace(results, hash, filePath.string()); // TODO: Allow per request download path
		cond.notify_all();
		return true;
	}

	void Exchanger::addLocalFile(Util::File file) {
		localFiles.emplace(file.hash, file);
	}

	void Exchanger::removeLocalFile(Util::File file) {
		localFiles.erase(file.hash);
	}

	void Exchanger::updateLocalFile(Util::File file) {
		localFiles.emplace(file.hash, file).first->second = file;
	}
}
