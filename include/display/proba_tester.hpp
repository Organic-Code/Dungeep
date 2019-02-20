#ifndef DUNGEEP_PROBA_TESTER_HPP
#define DUNGEEP_PROBA_TESTER_HPP

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


#include <imgui.h>
#include <vector>
#include <functional>
#include <string>

class proba_tester
{

public:
	explicit proba_tester() noexcept = default;

	void test_distribution(std::function<int()> random_engine, int min_, int max_, unsigned int step_ = 10) noexcept {
		distrib = std::move(random_engine);
		values.clear();
		values.resize(static_cast<unsigned>(max_ - min_ + 1), 0);
		min = min_;
		max = max_;
		step = step_;
        total_count = 0u;
	}

	void update() {
		for (auto i = step ; i != 0 ; --i) {
			int val = distrib();
			if (val >= min && val <= max) {
				++values[static_cast<unsigned>(val - min)];
			}
		}
        total_count += step;
        std::string label = "distribution: step " + std::to_string(total_count);
		ImGui::PlotHistogram(label.data(), values.data(), static_cast<int>(values.size()), 0, nullptr, 0, FLT_MAX, ImVec2(800, 200));
	}

private:
	std::function<int()> distrib{};
	std::vector<float> values{};
	int min{}, max{};
	unsigned int step{};
    unsigned int total_count{};
};


#endif //DUNGEEP_PROBA_TESTER_HPP
