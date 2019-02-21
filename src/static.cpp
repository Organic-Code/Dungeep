///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2019, Lucas Lazare                                                                                                     ///
///  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation         ///
///  files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,         ///
///  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software     ///
///  is furnished to do so, subject to the following conditions:                                                                        ///
///                                                                                                                                     ///
///  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.     ///
///                                                                                                                                     ///
///  The Software is provided “as is”, without warranty of any kind, express or implied, including but not limited to the               ///
///  warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or              ///
///  copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise,        ///
///  arising from, out of or in connection with the software or the use or other dealings in the Software.                              ///
///                                                                                                                                     ///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Insuring proper initialization order for static variables

#include "utils/resource_manager.hpp"
#include "utils/logger.hpp"

std::shared_ptr<logger::storing_sink> logger::sink{std::make_shared<logger::storing_sink>()};
spdlog::logger logger::log{"Dungeep", sink};

namespace {
	// Setting logger's verbosity
	struct logger_init_format {
		logger_init_format() noexcept {
			logger::log.set_level(spdlog::level::trace);
		}
	};
	[[maybe_unused]] logger_init_format _;
}

// resource manager can use logger anywhere
resources resources::manager{};