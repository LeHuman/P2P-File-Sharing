#pragma once

#include <fstream>
#include <vector>

#include <nlohmann/json.hpp>

#include "Index.h"

namespace Config {

	using nlohmann::json;
	using std::string;
	using std::vector;
	using Index::conn_t;

	struct config_t {
		uint32_t id;
		string ip;
		uint16_t port;
		conn_t server;
		bool isSuper;
		vector<conn_t> neighbors;
		int totalSupers;
	};
	using config_t = struct config_t;

	json loadConfig(string configPath);

	conn_t getConn(json::value_type &connection);

	config_t getConfig(uint32_t id, string configPath);
}