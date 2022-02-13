// Exchanger.cpp : Source file for your target.
//

#include <iostream>
#include <cstdlib>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <cstdio>
#include <thread>
#include <string>
#include <queue>

#include <co_spawn.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/connect.hpp>
#include <asio/detached.hpp>
#include <asio/awaitable.hpp>
#include <asio/signal_set.hpp>

#include "Exchanger.h"
#include "Log.h"

namespace Exchanger {
	using std::thread;
	using asio::ip::tcp;
#if defined(ASIO_HAS_CO_AWAIT)
	using asio::awaitable;
	using asio::co_spawn;
	using asio::detached;
#endif // defined(ASIO_HAS_CO_AWAIT)

	static const string ID = "Socket";

	enum {
		max_length = 4096
	};

	void fileSender(tcp::socket socket) {
		try {
			char buf[max_length];
			asio::error_code error;

			uint32_t *__id = (uint32_t *)buf;
			socket.read_some(asio::buffer(buf), error);
			uint32_t id = *__id;

			asio::write(socket, asio::buffer(buf, max_length)); // Write file size

			while (true) {
				if (error == asio::error::eof)
					break; // Connection closed cleanly by peer.
				else if (error)
					throw asio::system_error(error); // Some other error.
				asio::write(socket, asio::buffer(buf, max_length)); // Write file chunk
				//std::size_t n = socket.read_some(asio::buffer(buf), error); // get confirmation
			}
		} catch (std::exception &e) {
			Log.e(ID, "Socket Exception: %s\n", e.what());
		}
	}

	void fileReceiver(tcp::socket socket) {
		try {
			char request[max_length];
			asio::write(socket, asio::buffer(request, 4));

			char reply[max_length];
			size_t reply_length = asio::read(socket, asio::buffer(reply, 4));
			std::cout.write(reply, reply_length);
		} catch (std::exception &e) {
			Log.e(ID, "Socket Exception: %s\n", e.what());
		}
	}

	void OnResolve(asio::error_code &err, tcp::resolver::results_type type) {
		if (!err) {
			std::cout << "resolved!";
		} else {
			std::cout << "error.";
		}
	}

	struct query_t {
		string ip;
		int port;
		query_t(string ip, int port) : ip { ip }, port { port }{};
	};

	static std::mutex mutex;
	static std::condition_variable cond;
	static std::queue<struct query_t> queries;

	awaitable<void> listener(uint16_t port) {
		auto executor = co_await asio::this_coro::executor;
		tcp::acceptor acceptor(executor, { tcp::v4(), port });
		while (true) {
			tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
			Log.d(ID, "p2p conn: %s:%d", socket.local_endpoint().address().to_string().data(), socket.local_endpoint().port());
			thread(fileSender, std::move(socket)).detach();
		}
	}

	awaitable<void> receiver() {
		auto executor = co_await asio::this_coro::executor;
		tcp::resolver resolver(executor);
		//asio::error_code error;

		while (true) {
			std::unique_lock<std::mutex> lock(mutex);
			if (cond.wait_for(lock, std::chrono::milliseconds(50), [&]() {return !queries.empty(); })) {
				const struct query_t query = queries.front();
				queries.pop();

				tcp::socket s(executor);
				asio::connect(s, resolver.resolve(query.ip, std::to_string(query.port)));
				thread(fileReceiver, std::move(s)).detach();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	void _startSocket(uint32_t id, uint16_t listeningPort) { // TODO: somthing with the id, double check that it is the id we expect?
		try {
			asio::io_context io_context(thread::hardware_concurrency());

			asio::signal_set signals(io_context, SIGINT, SIGTERM);
			signals.async_wait([&](auto, auto) { io_context.stop(); });

			co_spawn(io_context, listener(listeningPort), detached);
			co_spawn(io_context, receiver(), detached);

			io_context.run();
		} catch (std::exception &e) {
			Log.e(ID, "Socket Exception: %s\n", e.what());
		}
	}

	void start(uint32_t id, uint16_t listeningPort) {
		thread(_startSocket, id, listeningPort).detach();
	}

	void connect(string ip, uint16_t port, entryHash_t hash) {
		std::lock_guard<std::mutex> lock(mutex);
		queries.emplace(ip, port);
		cond.notify_all();
	}

	void connect(Index::conn_t conn, entryHash_t hash) {
		connect(conn.ip, conn.port, hash);
	}
}
