/**
 * @file Config.h
 * @author IR
 * @brief Configuration module used to load the static config file
 * @version 0.1
 * @date 2022-03-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <fstream>
#include <vector>

#include <nlohmann/json.hpp>

#include "Index.h"

namespace Config {

	using Index::conn_t;
	using nlohmann::json;
	using std::string;
	using std::vector;

	/**
	 * @brief Struct used to store configuration details
	 */
	struct config_t {
		uint32_t id;
		string ip;
		uint16_t port;
		conn_t server;
		bool all2all;
		bool isSuper;
		vector<conn_t> neighbors;
		int totalSupers;
	};
	using config_t = struct config_t;

	/**
	 * @brief Get the configuration for this peer
	 *
	 * @param id Unique ID of this peer, must be defined in the JSON
	 * @param configPath The path to the config file
	 * @param getAllNeighbors Whether to load all neighbors into the config for all2all mode
	 * @return config_t configuration struct
	 */
	config_t getConfig(uint32_t id, string configPath, bool getAllNeighbors = false);

} // namespace Config