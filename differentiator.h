/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <array>

template <typename FloatType>
class Differentiator
{
private:
    static constexpr FloatType coeffs[] {
        0.0209,
        0.0,
        -0.1128,
        0.0,
        1.2411,
        0.0,
        -1.2411,
        0.0,
        0.1128,
        0.0,
        -0.0209
    };

    static constexpr size_t length{sizeof(coeffs) / sizeof(FloatType)};
    std::array<FloatType, length> history;
    size_t index{length - 1};

public:
    static constexpr size_t delayTime{(length - 1)  / 2};
    Differentiator()
    {
        history.fill(0.0);
	}

	FloatType get(const FloatType& input)
	{
		// place input into history
        history[index] = input;

		// perform the convolution
		FloatType dP{0.0}; // differentiator result
        size_t p = index;
        for(size_t j = 0 ; j < length; j++) {
            dP += coeffs[j] * history.at(p);
            if(++p == length) {
                p = 0; // wrap
			}
		}

		// update the current index
        if(index == 0) {
            index = length - 1; // wrap
		} else {
            index--;
		}

        return dP;
    }
};

#endif // DIFFERENTIATOR_H
