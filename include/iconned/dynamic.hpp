#ifndef DUNGEEP_DYNAMIC_HPP
#define DUNGEEP_DYNAMIC_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                     ///
///  Copyright C 2018, Lucas Lazare                                                                                                     ///
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

#include <memory>

#include "iconned/fixed.hpp"

namespace Json {
	class Value;
}

class player;
class world_proxy;

class dynamic_effect : public fixed_effect {
public:
	virtual int update_cooldown_once(int current_cooldown) noexcept = 0;
	virtual int update_input_physical_damage(int input) noexcept = 0;
	virtual int update_input_magical_damage(int input) noexcept = 0;
	virtual int update_input_true_damage(int input) noexcept = 0;
	virtual int update_output_physical_damage(int input) noexcept = 0;
	virtual int update_output_magical_damage(int input) noexcept = 0;
	virtual int update_output_true_damage(int input) noexcept = 0;
	virtual int ignored_armor(int target_armor) noexcept = 0;
	virtual int ignored_resist(int target_resist) noexcept = 0;

	virtual void tick(player&, world_proxy&) noexcept = 0;

	// TODO should be final and not override, but compiler is stupid.
	bool is_dynamic() const noexcept override {
		return true;
	}

	static std::unique_ptr<dynamic_effect> find_by_name(std::string&& /*name*/, fixed_effect::defense
			, fixed_effect::attack, fixed_effect::critics, fixed_effect::misc, const Json::Value&) { return nullptr; /* TODO */}
};

#endif //DUNGEEP_DYNAMIC_HPP
