#include "Config.h"

namespace Config {

	json loadConfig(string configPath) {
		json config;
		std::ifstream f(configPath);

		f >> config;
		return config;
	}

	conn_t getConn(json::value_type &connection) {
		return conn_t(connection["ip"], connection["port"]);
	}

	config_t getConfig(uint32_t id, string configPath) {
		json configJson = loadConfig(configPath);

		json::value_type &relations = configJson["relations"];
		json::value_type &connections = configJson["connections"];
		json::value_type &our = connections[id];
		json::value_type ourRelation;

		if (our["id"] != id)
			throw new std::runtime_error("id not found");

		config_t config;

		config.id = id;
		config.ip = our["ip"];
		config.port = our["port"];
		config.isSuper = our["type"] == "super";
		config.totalSupers = relations.size();

		if (config.isSuper) {
			ourRelation = relations[id];

			if (ourRelation["id"] != id) {
				throw new std::runtime_error("super id not found");
			}

			vector<uint32_t> neighborIDs = ourRelation["neighbors"];
			for (uint32_t i : neighborIDs) {
				config.neighbors.push_back(getConn(connections[i]));
			}
		} else {
			for (auto &relation : relations) {
				for (uint32_t peer : relation["peers"]) {
					if (peer == id) {
						ourRelation = relation;
						break;
					}
				}
				if (ourRelation == relation)
					break;
			}

		}

		config.server = getConn(connections[(uint32_t)ourRelation["id"]]);

		return config;
	}
}