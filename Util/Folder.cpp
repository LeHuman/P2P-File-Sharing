#include <thread>
#include <chrono>
#include <fstream>
#include <iterator>
#include <vector>

#include "Folder.h"
#include "Log.h"
#include "picosha2.h"

namespace Util {
	File::File(string path) {
		this->path = fs::path(path);
		if (!fs::is_regular_file(this->path) || !fs::exists(this->path)) {
			throw std::runtime_error("File is not regular or does not exist");
		}
		name = this->path.filename().string();
		time = fs::last_write_time(this->path);
		size = fs::file_size(path);
		hash = getHash();
	}

	void File::update(fs::file_time_type time, uintmax_t size) {
		this->time = time;
		this->size = size;
		prehash = hash;
		hash = getHash();
	}

	string File::getHash() {
		std::ifstream f(path, std::ios::binary);
		std::vector<unsigned char> hash(picosha2::k_digest_size);
		picosha2::hash256(f, hash.begin(), hash.end());
		return picosha2::bytes_to_hex_string(hash.begin(), hash.end()); // TODO: Hash function is *very* slow
	}

	void Folder::operator()(fs::path path, std::chrono::duration<int, std::milli> delay, const std::function<void(File, File::Status)> &listener) {
		while (running) {
			auto it = files.begin();
			while (it != files.end()) {
				if (!fs::exists(it->first)) {
					listener(it->second, File::Status::erased);
					it = files.erase(it);
				} else {
					it++;
				}
			}
			for (auto &file : fs::recursive_directory_iterator(path)) {
				auto current_file_last_write_time = fs::last_write_time(file);
				string path = file.path().string();
				if (!files.contains(path)) {
					files[path] = File(file.path(), current_file_last_write_time, file.file_size());
					listener(files[path], File::Status::created);
				} else {
					if (files[path].time != current_file_last_write_time) {
						files[path].update(current_file_last_write_time, file.file_size());
						listener(files[path], File::Status::modified);
					}
				}
			}
			std::this_thread::sleep_for(delay);
		}
	}

	std::thread *current = nullptr;

	void watchFolder(string path, int delay, const std::function<void(File, File::Status)> &listener) {
		if (current != nullptr) {
			Log.w("Folder", "Already watching a folder");
			return;
		}
		Folder fld = Folder();
		current = new std::thread(fld, path, std::chrono::duration<int, std::milli>(delay), listener);
	}

	void stopFolder() {
		if (current != nullptr) {
			delete current;
		}
	}
}
