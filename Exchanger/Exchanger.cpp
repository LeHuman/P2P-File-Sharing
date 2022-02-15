// Exchanger.cpp : Source file for your target.
//

#include <iostream>
#include <cstdlib>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include <queue>
#include <fstream>

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

	void Exchanger::fileSender(int id, tcp::iostream stream) {
		State state = State::S_Accept;
		std::stringbuf hBuf;
		asio::error_code error;
		Index::entryHash_t hash;
		Util::File file;
		std::ifstream fileOut;
		std::unordered_map<Index::entryHash_t, Util::File>::const_iterator it;

		std::streamsize written = 1;
		char sBuf[block_size] = { 0 };

		try {
			while (state != State::Disconnect) {
				switch (state) {
					case State::S_Accept:
						stream << id; // Send out ID so Client can confirm
						state = State::S_FindFile;
						break;
					case State::S_FindFile:
						stream >> &hBuf;
						hash = (Index::entryHash_t)hBuf.str(); // Get Hash from Client

						if ((it = localFiles.find(hash)) != localFiles.end()) { // File exists, open file and send size of file
							file = it->second;
							fileOut = std::ifstream(file.path, std::ios_base::in | std::ios_base::binary); // TODO: ensure file has not changed, confirm localFiles matches details
							stream << (uint64_t)file.size;
							state = State::Streaming;
						} else { // File does not exist, disconnect
							stream << 0;
							state = State::Disconnect;
						}
						break;
					case State::Streaming:
						if (stream.get()) { // Get confirmation from client
							while (written > 0 && !fileOut.eof() && !fileOut.bad() && !stream.error()) {
								written = fileOut.readsome(sBuf, block_size);
								stream.write(sBuf, written);
							}
						}
						state = State::Disconnect;
						break;
					default:
						Log.e(ID, "Unknown server connection state");
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
			Log.e(ID, "Socket Server Exception: %s\n", e.what());
			stream.close();
		}
	}

	void Exchanger::fileReceiver(tcp::iostream stream, int eid, entryHash_t hash, string filePath) {
		State state = State::C_ConfirmID;
		uint64_t fileSize = 0;
		asio::error_code error;
		std::ofstream fileIn;

		std::streamsize received = 1;
		char sBuf[block_size] = { 0 };

		try {
			while (state != State::Disconnect) {
				switch (state) {
					case State::C_ConfirmID:
						if (stream.get() != eid) {
							state = State::Disconnect;
						} else {
							stream << hash;
							state = State::C_AllocSpace;
						}
						break;
					case State::C_AllocSpace:
						stream.read((char *)&fileSize, sizeof(uint64_t));
						if (fileSize == 0) { // TODO: Check file size for mistmatch and also allocate file?
							stream << 0;
							state = State::Disconnect;
						} else {
							fileIn = std::ofstream(filePath, std::ios_base::out | std::ios_base::binary);
							stream << 1;
							state = State::Streaming;
						}
						break;
					case State::Streaming:
						while (received > 0 && !fileIn.bad() && !stream.error()) {
							received = stream.readsome(sBuf, block_size);
							fileIn.write(sBuf, received);
						}
						state = State::Disconnect;
						break;
					default:
						Log.e(ID, "Unknown client connection state");
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
			Log.e(ID, "Socket Client Exception: %s\n", e.what());
			stream.close();
		}
	}

	void OnResolve(asio::error_code &err, tcp::resolver::results_type type) {
		if (!err) {
			std::cout << "resolved!";
		} else {
			std::cout << "error.";
		}
	}

	void Exchanger::listener(int id, uint16_t port) {
		tcp::acceptor acceptor(*io_context, tcp::endpoint(tcp::v4(), port));

		while (true) {
			tcp::iostream stream;
			acceptor.accept(stream.socket());

			auto endpnt = stream.socket().local_endpoint();
			Log.d(ID, "p2p conn: %s:%d", endpnt.address().to_string().data(), endpnt.port());
			thread(&Exchanger::fileSender, this, id, std::move(stream)).detach();
		}
	}

	void Exchanger::receiver() { // TODO: set timeout
		tcp::resolver resolver(*io_context);
		while (running) {
			std::unique_lock<std::mutex> lock(mutex);
			if (cond.wait_for(lock, std::chrono::milliseconds(50), [&]() {return !queries.empty(); })) {
				const struct query_t query = queries.front();
				queries.pop();

				tcp::iostream stream(query.ip, std::to_string(query.port));
				thread(&Exchanger::fileReceiver, this, std::move(stream), query.id, query.hash, query.filePath).detach();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		running = true;
	}

	void Exchanger::_startSocket(int id, uint16_t listeningPort) {
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
		}
	}

	Exchanger::Exchanger(int id, uint16_t listeningPort) {
		io_context = new asio::io_context(thread::hardware_concurrency());
		thread(&Exchanger::_startSocket, this, id, listeningPort).detach();
	}

	void Exchanger::connect(int id, string ip, uint16_t port, entryHash_t hash, string filePath) {
		std::lock_guard<std::mutex> lock(mutex);
		queries.emplace(id, ip, port, hash, filePath);
		cond.notify_all();
	}

	void Exchanger::connect(Index::conn_t conn, int id, entryHash_t hash, string filePath) { // TODO: make conn_t have everything
		connect(id, conn.ip, conn.port, hash, filePath);
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
