// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <chrono>
#include <ctime>

#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "concurrent_queue.h"
#include "log_ostream.h"
#include "log_stream_formatter.h"
#include "log_sink.h"
#include "log_class.h"

#include "gl_utils.h"

namespace StE {

class Logger {
private:
	friend class Log;

private:
	Logger(const std::shared_ptr<log_sink> &sink, bool force_flush = false) : stream(sink, force_flush) {}

	log_ostream stream;

public:
	log_ostream &logger() { return stream; }
	~Logger() {}
};

class Log {
private:
	using queue_type = concurrent_queue<log_entry>;

private:
	log_stream_formatter formatter;

	std::shared_ptr<std::condition_variable> notifier;
	std::shared_ptr<queue_type> queue;
	std::mutex m;
	std::thread t;
	std::atomic<bool> finish;

	std::string file_path;

	std::string file_name(const std::string &title) {
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[256];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 256, "%a %d%b%y %H.%M.%S", timeinfo);

		return title + " " + buffer;
	}

	void dump() {
		std::unique_ptr<log_entry> entry;
		while ((entry = queue->pop()) != nullptr)
			write_entry(std::move(entry));
	}

	void write_entry(std::unique_ptr<log_entry> entry) {
		auto fs = std::ofstream();
		fs.open(file_path, std::ofstream::app);
		auto line = formatter.format_line(*entry);
		fs << line << std::endl;
	}

	void write_head() {
		std::ofstream fs;
		fs.open(file_path, std::ofstream::trunc);
		fs << formatter.format_head();
	}

	void write_tail() {
		std::ofstream fs;
		fs.open(file_path, std::ofstream::app);
		fs << formatter.format_tail();
	}

	std::streambuf *cout_strm_buffer;
	std::streambuf *cerr_strm_buffer;
	std::unique_ptr<Logger> cout_logger;
	std::unique_ptr<Logger> cerr_logger;

public:
	Log(const std::string &title, const std::string path_prefix = R"(Log\)", const std::string path_extension = ".html") :
			formatter(title),
			notifier(std::make_shared<std::condition_variable>()),
			queue(std::make_shared<queue_type>()),
			finish(false),
			cout_strm_buffer(nullptr),
			cerr_strm_buffer(nullptr) {
		file_path = path_prefix + file_name(title) + path_extension;

		write_head();

		t = std::thread([this] {
			for (;;) {
				std::unique_lock<std::mutex> ul(m);
				dump();

				notifier->wait(ul);
				if (finish.load()) {
					finish.store(false);
					return;
				}
			}
		});
	}

	~Log() {
		std::unique_lock<std::mutex> ul(m);

		if (cout_logger != nullptr) cout_logger->stream.flush();
		if (cerr_logger != nullptr) cerr_logger->stream.flush();
		dump();

		// Notify worker that we are done
		finish.store(true);
		ul.unlock();
		// Wait for worker to roger that
		while (finish.load())
			notifier->notify_one();

		write_tail();

		if (cout_strm_buffer) std::cout.rdbuf(cout_strm_buffer);
		if (cerr_strm_buffer) std::cerr.rdbuf(cerr_strm_buffer);
		cout_strm_buffer = cerr_strm_buffer = nullptr;

		if (t.joinable())
			t.join();
	}

	void redirect_std_outputs() {
		cout_strm_buffer = std::cout.rdbuf();
		cerr_strm_buffer = std::cerr.rdbuf();
		cout_logger = std::unique_ptr<Logger>(new Logger(std::make_shared<log_sink>(log_entry_data("std", "std::cout", 0, log_class::info_class_log), notifier, queue)));
		cerr_logger = std::unique_ptr<Logger>(new Logger(std::make_shared<log_sink>(log_entry_data("std", "std::cerr", 0, log_class::err_class_log), notifier, queue), true));

		std::cout.rdbuf(cout_logger->logger().rdbuf());
		std::cerr.rdbuf(cerr_logger->logger().rdbuf());
	}

	Logger log_info(const char *file, const char *func, int line) {
		return Logger(std::make_shared<log_sink>(log_entry_data(file, func, line, log_class::info_class_log), notifier, queue));
	}
	Logger log_warn(const char *file, const char *func, int line) {
		return Logger(std::make_shared<log_sink>(log_entry_data(file, func, line, log_class::warn_class_log), notifier, queue));
	}
	Logger log_err(const char *file, const char *func, int line) {
		return Logger(std::make_shared<log_sink>(log_entry_data(file, func, line, log_class::err_class_log), notifier, queue), true);
	}
	Logger log_fatal(const char *file, const char *func, int line) {
		return Logger(std::make_shared<log_sink>(log_entry_data(file, func, line, log_class::fatal_class_log), notifier, queue), true);
	}
};

extern Log *ste_global_logger;

}

void inline ste_log_set_global_logger(StE::Log *ptr) {
	StE::ste_global_logger = ptr;
}

#define ___STE_LOG_EVERY_VAR(x,l) ___STE_LOG_VAR ## x ## l
#define ___STE_LOG_EVERY(func, n) static std::chrono::time_point< std::chrono::high_resolution_clock> ___STE_LOG_EVERY_VAR(1,__LINE__); bool ___STE_LOG_EVERY_VAR(2,__LINE__) = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - ___STE_LOG_EVERY_VAR(1,__LINE__)) > static_cast<std::chrono::milliseconds>(n); if (___STE_LOG_EVERY_VAR(2,__LINE__)) ___STE_LOG_EVERY_VAR(1,__LINE__) = std::chrono::high_resolution_clock::now(); if (___STE_LOG_EVERY_VAR(2,__LINE__)) func

#define __ste_log_null	(std::cout)

#define ste_log()		(StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_info(__FILE__,__func__,__LINE__).logger() : __ste_log_null)
#define ste_log_warn()	(StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_warn(__FILE__,__func__,__LINE__).logger() : __ste_log_null)
#define ste_log_error()	(StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_err(__FILE__,__func__,__LINE__).logger() : __ste_log_null)
#define ste_log_fatal() (StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_fatal(__FILE__,__func__,__LINE__).logger() : __ste_log_null)

// Write to info log, limit log to one per n milliseconds
#define ste_log_every(n)		___STE_LOG_EVERY((StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_info(__FILE__,__func__,__LINE__).logger() : __ste_log_null),n)
// Write to warning log, limit log to one per n milliseconds
#define ste_log_every_warn(n)	___STE_LOG_EVERY((StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_warn(__FILE__,__func__,__LINE__).logger() : __ste_log_null),n)
// Write to error log, limit log to one per n milliseconds
#define ste_log_every_error(n)	___STE_LOG_EVERY((StE::ste_global_logger!=nullptr ? StE::ste_global_logger->log_err(__FILE__,__func__,__LINE__).logger() : __ste_log_null),n)

// Query and log gl errors
#define ste_log_query_and_log_gl_errors()	{std::string gl_err_desc; while (StE::LLR::opengl::query_gl_error(gl_err_desc)) ste_log_error() << "OpenGL Error: " << gl_err_desc << std::endl;}
