/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <array>
#include <QVector>

template <typename FloatType>
class Interpolator
{
private:
	static constexpr FloatType coeffs[] {
		-0.00087560000,
		-0.0011643619,
		0.0010874736,
		0.0022780589,
		-0.0035814886,
		-0.0036865168,
		0.0068853690,
		0.027796547,
		-0.11749555,
		0.22353046,
		0.72835886,
		0.22353046,
		-0.11749555,
		0.027796547,
		0.0068853690,
		-0.0036865168,
		-0.0035814886,
		0.0022780589,
		0.0010874736,
		-0.0011643619,
		-0.00087560000
	};

    static constexpr size_t length{sizeof(coeffs) / sizeof(FloatType)};
    std::array<FloatType, length> history;
    size_t index{length - 1};

public:
    static constexpr size_t delayTime{(length - 1)  / 2};
	Interpolator()
    {
        history.fill(0.0);
	}

	QVector<FloatType> blockGet(const QVector<FloatType>& input, size_t length)
	{

		QVector<FloatType> output;
		output.reserve(2 * input.length());
		for (size_t i = 0; i < length; i++) {
			output.append(get(input));
			output.append(get(0.0));
		}
		return output;
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

#endif // INTERPOLATOR_H
