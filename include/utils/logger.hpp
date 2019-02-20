///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  		files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,  ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  		is furnished to do so, subject to the following conditions:                                                                 ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DUNGEEP_LOGGER_HPP
#define DUNGEEP_LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace logger {

	struct message {
		spdlog::level::level_enum severity;
		size_t color_beg;
		size_t color_end;
		std::string value;
	};

	class storing_sink final : public spdlog::sinks::base_sink<std::mutex> {
	public:
		auto begin() const {
			return messages.begin();
		}

		auto end() const {
			return messages.end();
		}

		auto size() const {
			return messages.size();
		}

		void force_add_message(message&& msg) {
			std::lock_guard lg(base_sink::mutex_);
			messages.emplace_back(std::move(msg));
		}

		void clear() {
			std::lock_guard lg(base_sink::mutex_);
			messages.clear();
		}

	protected:
		void sink_it_(const spdlog::details::log_msg &msg) override {
			fmt::memory_buffer buff;
			sink::formatter_->format(msg, buff);
			messages.push_back(message{msg.level, msg.color_range_start, msg.color_range_end, fmt::to_string(buff)});
		}

		void flush_() override {}

	private:
		std::vector<message> messages{};
	};


	extern spdlog::logger log;
	extern std::shared_ptr<logger::storing_sink> sink;
}


#endif //DUNGEEP_LOGGER_HPP
