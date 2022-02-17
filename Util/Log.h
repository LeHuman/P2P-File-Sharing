#pragma once

#include <string>
#include <mutex>
#include <iostream>

#include <stdio.h>

namespace Util {
	namespace Logging {
		using std::string;

		static const string C_RED = "\x1b[1;31m";
		static const string C_GREEN = "\x1b[1;32m";
		static const string C_YELLOW = "\x1b[1;33m";
		static const string C_BLUE = "\x1b[1;34m";
		static const string C_MAGENTA = "\x1b[1;35m";
		static const string C_CYAN = "\x1b[1;36m";
		static const string C_GREY = "\x1b[1;30m";
		static const string C_UNDER = "\x1b[4m";
		static const string C_RESET = "\x1b[0m";

		static const string L_DEBUG = C_MAGENTA + "DEBUG" + C_RESET + "  ";
		static const string L_INFO = C_BLUE + "INFO" + C_RESET + "   ";
		static const string L_WARNING = C_YELLOW + "WARNING" + C_RESET;
		static const string L_ERROR = C_RED + "ERROR" + C_RESET + "  ";
		static const string L_FATAL = C_UNDER + C_RED + "FATAL" + C_RESET + "  ";

		/**
		 * @brief This struct enables an easy way to log, organize and color messages on the terminal
		*/
		struct Log_t {
		private:
			std::mutex print_l;
			template <typename ... Args>
			void __log(string ID, string level, char const *const format, Args ... args) {
				size_t sz = (size_t)snprintf(NULL, 0, format, args ...) + 1;
				char *buf = new char[sz];
				snprintf(buf, sz, format, args ...);
				print_l.lock();
				std::cout << C_GREY << "[" << ID << "] " << C_RESET << level << ": " << buf << std::endl;
				std::cout << std::flush;
				print_l.unlock();
				delete[] buf;
			}
		public:
			template <typename ... Args>
			void operator()(string ID, char const *const format, Args ... args) {
				__log(ID, "       ", format, args ...);
			}
			template <typename ... Args>
			void d(string ID, char const *const format, Args ... args) {
				__log(ID, L_DEBUG, format, args ...);
			}
			template <typename ... Args>
			void i(string ID, char const *const format, Args ... args) {
				__log(ID, L_INFO, format, args ...);
			}
			template <typename ... Args>
			void w(string ID, char const *const format, Args ... args) {
				__log(ID, L_WARNING, format, args ...);
			}
			template <typename ... Args>
			void e(string ID, char const *const format, Args ... args) {
				__log(ID, L_ERROR, format, args ...);
			}
			template <typename ... Args>
			void f(string ID, char const *const format, Args ... args) {
				__log(ID, L_FATAL, format, args ...);
			}
			/*void test() {
				operator()("Test", "Oui");
				d("Test", "This");
				i("Test", "Is");
				w("Test", "A");
				e("Test", "Test");
				f("Test", "See?");
			}*/
		};
	}
}

/**
 * @brief Logging object that can be used globally
*/
extern struct Util::Logging::Log_t Log;
