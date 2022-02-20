#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>
#include <thread>
#include <functional>

namespace Util {
	using std::string;
	namespace fs = std::filesystem;

	/**
	 * @brief Helper represents a file, it ensures a file exists and hashes it on creation
	*/
	struct File {
		fs::path path;
		fs::file_time_type time;
		string name;
		string hash;
		string prehash;
		uintmax_t size = 0;

		struct not_regular_error : public std::exception {
			const char *what() const throw () {
				return "File is not regular or does not exist";
			}
		};

		enum class Status {
			created, modified, erased
		};

		File() {}
		File(string path);
		File(fs::path path, fs::file_time_type time, uintmax_t size) : path { path }, time { time }, size { size }, name { path.filename().string() }, hash { getHash() }{}

		/**
		 * @brief Update this file's time, size and hash, used when modified
		 * @param time The new time-stamp of the file
		 * @param size The new size of the file
		*/
		void update(fs::file_time_type time, uintmax_t size);

		/**
		 * @brief Compute the hash of this file
		 * @note Use the hash parameter to get the saved hash
		 * @return The hash of this file as a string
		*/
		string getHash();
	};

	/**
	 * @brief Class that continuously monitors a folder and calls a listener function on changes
	*/
	class Folder {
		std::unordered_map<string, File> files;
		bool running = true;

		void run(fs::path path, const std::function<void(File, File::Status)> &listener, std::chrono::duration<int, std::milli> delay);

	public:
		~Folder();

		/**
		 * @brief Creates a new folder watcher
		 * @param path The directory to watch
		 * @param listener The listener function to run on changes
		 * @param delay The Delay between updates in milliseconds
		*/
		Folder(string path, const std::function<void(File, File::Status)> &listener, int delay = 1000);

		/**
		 * @brief Stop this folder watcher from running
		*/
		void stop();
	};
}
