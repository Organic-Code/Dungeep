#ifndef DUNGEEP_RESOURCE_MANAGER_HPP
#define DUNGEEP_RESOURCE_MANAGER_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
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


#include <utility>
#include <vector>
#include <string_view>
#include <json/json.h>

#include "map.hpp"

class resource_manager_t {
public:
	resource_manager_t() noexcept;

	const std::vector<std::string>& get_map_list() const;

	std::tuple<map::size_type, std::vector<room_gen_properties>, hallway_gen_properties> get_map(std::string_view map_name) const;

	void save_map(std::string_view map_name, map::size_type size, const std::vector<room_gen_properties>& room_properties, const hallway_gen_properties& halls_properties);

private:

	Json::Value maps{};
	std::vector<std::string> map_list{};
};

extern resource_manager_t resource_manager;

#endif //DUNGEEP_RESOURCE_MANAGER_HPP
