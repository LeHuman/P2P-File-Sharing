#include <thread>
#include <chrono>
#include <fstream>
#include <iterator>
#include <vector>

#include "cryptlib.h"
#include "channels.h"
#include "filters.h"
#include "files.h"
#include "sha.h"
#include "hex.h"

#include "Folder.h"
#include "Log.h"

namespace Util {
	File::File(string path) {
		this->path = fs::path(path);
		if (!fs::is_regular_file(this->path) || !fs::exists(this->path)) {
			throw not_regular_error();
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
		Log.d("File", "Hashing file: %s", name.data());

		std::string _hash;
		CryptoPP::SHA256 sha256;

		CryptoPP::HashFilter hf(sha256, new CryptoPP::HexEncoder(new CryptoPP::StringSink(_hash)));

		CryptoPP::ChannelSwitch cs;
		cs.AddDefaultRoute(hf);

		auto fs = CryptoPP::FileSource(path.string().data(), true /*pumpAll*/, new CryptoPP::Redirector(hf));
		return _hash;
	}

	void Folder::run(fs::path path, const std::function<void(File, File::Status)> &listener, std::chrono::duration<int, std::milli> delay) {
		while (running) { // TODO: ignore files that are being downloaded
			auto it = files.begin();
			while (it != files.end()) {
				if (!fs::exists(it->first)) {
					listener(it->second, File::Status::erased);
					it = files.erase(it);
				} else {
					it++;
				}
			}
			for (auto &file : fs::recursive_directory_iterator(path)) { // TODO: concurrent file indexing
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
		Log.d("Folder", "Stopped watching path for changes");
		running = true;
	}

	Folder::Folder(string path, const std::function<void(File, File::Status)> &listener, int delay) {
		std::thread(&Folder::run, this, path, listener, std::chrono::duration<int, std::milli>(delay)).detach();
	}

	Folder::~Folder() {
		stop();
	}

	void Folder::stop() {
		running = false;
		while (running == false) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
