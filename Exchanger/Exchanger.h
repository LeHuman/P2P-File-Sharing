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
		Index::PeerResults results;
		string downloadPath;
		entryHash_t hash;
		query_t(Index::PeerResults results, string hash, string downloadPath) : results { results }, hash { hash }, downloadPath { downloadPath }{};
	};

	class Exchanger {
		asio::io_context *io_context = nullptr;

		bool running = true;

		std::mutex mutex;
		string downloadPath;
		std::condition_variable cond;
		std::queue<struct query_t> queries;
		std::unordered_map<Index::entryHash_t, Util::File> localFiles;

		void fileSender(uint32_t id, asio::ip::tcp::iostream stream);
		bool fileReceiver(asio::ip::tcp::iostream &stream, uint32_t eid, entryHash_t hash, string downloadPath);
		void listener(uint32_t id, uint16_t port);
		void peerResolver(Index::PeerResults peers, Index::entryHash_t hash, string downloadPath);
		void receiver();
		void _startSocket(uint32_t id, uint16_t listeningPort);

	public:
		~Exchanger();
		Exchanger(uint32_t id, uint16_t listeningPort, string downloadPath);

		void stop();

		void setDownloadPath(string downloadPath);

		void download(Index::PeerResults peers, entryHash_t hash);

		void addLocalFile(Util::File file);

		void removeLocalFile(Util::File file);

		void updateLocalFile(Util::File file);
	};
}
