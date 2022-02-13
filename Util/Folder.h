#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>
#include <thread>
#include <functional>

namespace Util {
	using std::string;
	namespace fs = std::filesystem;

	struct File {
		fs::path path;
		fs::file_time_type time;
		string hash;
		string prehash;
		uintmax_t size = 0;

		enum class Status {
			created, modified, erased
		};

		File() {}
		File(fs::path path, fs::file_time_type time, uintmax_t size) : path { path }, time { time }, size { size }, hash { getHash() }{}

		void update(fs::file_time_type time, uintmax_t size);

		string getHash();
	};

	class Folder {
		std::unordered_map<string, File> files;

	public:
		bool running = true;

		void operator()(fs::path path, std::chrono::duration<int, std::milli> delay, const std::function<void(File, File::Status)> &listener);
	};

	void watchFolder(string path, int delay, const std::function<void(File, File::Status)> &listener);
}