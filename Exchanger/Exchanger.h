// Exchanger.h : Header file for your target.

#pragma once

#include <stdint.h>
#include <string>
#include <queue>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "index.h"
#include "Folder.h"

namespace Exchanger {
	using std::string;
	using Index::entryHash_t;

	struct query_t {
		int id;
		int port;
		string ip;
		string filePath;
		entryHash_t hash;
		query_t(int id, string ip, int port, string hash, string filePath) : id { id }, ip { ip }, port { port }, hash { hash }, filePath { filePath }{};
	};

	class Exchanger {
		asio::io_context *io_context = nullptr;

		bool running = true;

		std::mutex mutex;
		std::condition_variable cond;
		std::queue<struct query_t> queries;
		std::unordered_map<Index::entryHash_t, Util::File> localFiles;

		void fileSender(int id, asio::ip::tcp::iostream stream);
		void fileReceiver(asio::ip::tcp::iostream stream, int eid, entryHash_t hash, string filePath);
		void listener(int id, uint16_t port);
		void receiver();
		void _startSocket(int id, uint16_t listeningPort);

	public:
		~Exchanger();
		Exchanger(int id, uint16_t listeningPort);

		void stop();

		void connect(Index::conn_t conn, int id, entryHash_t hash, string filePath);

		void connect(int id, string ip, uint16_t port, entryHash_t hash, string filePath);

		void addLocalFile(Util::File file);

		void removeLocalFile(Util::File file);

		void updateLocalFile(Util::File file);
	};
}
