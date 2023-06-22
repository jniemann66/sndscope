/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef DIFFERENTIATOR_H
#define DIFFERENTIATOR_H

#include <vector>

template <typename FloatType>
class Differentiator
{

private:
	std::vector<FloatType> history;
	std::vector<FloatType> coeffs{
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
	size_t differentiatorLength{coeffs.size()};
	size_t differentiatorDelay{differentiatorLength / 2};
	size_t differentiatorIndex{differentiatorLength - 1};

public:
	Differentiator()
	{
		history.resize(differentiatorLength, 0.0);
		differentiatorDelay = differentiatorLength / 2;
	}

	FloatType get(const FloatType& input)
	{
		// place input into history
		history[differentiatorIndex] = input;

		// perform the convolution
		FloatType dP{0.0}; // differentiator result
		size_t p = differentiatorIndex;
		for(size_t j = 0 ; j < differentiatorLength; j++) {
			FloatType vP = history.at(p);
			if(++p == differentiatorLength) {
				p = 0; // wrap
			}
			dP += coeffs[j] * vP;
		}

		// update the current index
		if(differentiatorIndex == 0) {
			differentiatorIndex = differentiatorLength - 1; // wrap
		} else {
			differentiatorIndex--;
		}

		return dP;
	}

};

#endif // DIFFERENTIATOR_H
