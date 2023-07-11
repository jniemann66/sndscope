/*
* Copyright (C) 2020 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/sndscope.git
*/

#ifndef UPSAMPLER_H
#define UPSAMPLER_H


// #define UPSAMPLER_TIME_FUNC

#ifdef UPSAMPLER_TIME_FUNC
#include "movingaverage.h"
#include "functimer.h"
#endif

#include <vector>

#include <QDebug>

template <typename InputType, typename OutputType, int L>
class UpSampler
{
private:
	static constexpr OutputType coeffs2[] {
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

	// todo: design filters for other L - factors
	static constexpr OutputType coeffs4[] { // Coeffs: 73
		9.962570624126234e-06,
		3.957986016501213e-05,
		7.897128934649362e-05,
		9.703385688654565e-05,
		4.907628580966033e-05,
		-9.745242907276185e-05,
		-0.0003277460000285871,
		-0.0005546006935996341,
		-0.0006271869492189099,
		-0.0003873942489772647,
		0.0002360729907676491,
		0.001134117122777654,
		0.001980393734487934,
		0.002299817644506438,
		0.001652311950832638,
		-0.000118129592931525,
		-0.002674473313982637,
		-0.005142797340279459,
		-0.006319879497941102,
		-0.005122599644451211,
		-0.001147119917015838,
		0.004890392747595472,
		0.01107576602347318,
		0.01473257464486369,
		0.01333568444250735,
		0.005648367147916558,
		-0.007330649408092939,
		-0.0220424891989685,
		-0.03294052256597733,
		-0.0339351609472972,
		-0.02034506079624187,
		0.009262557923020177,
		0.0520089438641673,
		0.1008883411385215,
		0.146278561186415,
		0.1784009001925697,
		0.19,
		0.1784009001925697,
		0.146278561186415,
		0.1008883411385215,
		0.0520089438641673,
		0.009262557923020177,
		-0.02034506079624187,
		-0.0339351609472972,
		-0.03294052256597733,
		-0.0220424891989685,
		-0.007330649408092947,
		0.005648367147916558,
		0.01333568444250735,
		0.01473257464486369,
		0.01107576602347318,
		0.004890392747595472,
		-0.001147119917015838,
		-0.005122599644451211,
		-0.006319879497941102,
		-0.005142797340279469,
		-0.002674473313982637,
		-0.000118129592931525,
		0.001652311950832638,
		0.002299817644506438,
		0.001980393734487934,
		0.001134117122777654,
		0.000236072990767649,
		-0.0003873942489772647,
		-0.0006271869492189109,
		-0.0005546006935996341,
		-0.0003277460000285872,
		-9.745242907276185e-05,
		4.907628580966033e-05,
		9.703385688654547e-05,
		7.897128934649362e-05,
		3.957986016501203e-05,
		9.962570624126234e-06
	};

public:
	// FIR Filter
	size_t fir_length;
	std::vector<OutputType> fir_coeffs;
	std::vector<OutputType> history0;
	std::vector<OutputType> history1;

	size_t index;
	size_t n;

	UpSampler()
    {
		switch(L) {
		case 1:
			break;
		case 2:
			setCoefficients(coeffs2);
			break;
		case 4:
			setCoefficients(coeffs4);
			break;
		default:
			setCoefficients(coeffs4);
			// caller must supply own coefficients (by calling setCoefficients( ...)
			break;
		}
	}

	// set coeffs from native array
	template<size_t Array_Length>
	void setCoefficients(const OutputType (&array)[Array_Length])
	{
		setCoefficients(array, Array_Length);
	}

	// set coeffs from pointer + count
	void setCoefficients(const OutputType* firCoeffs, size_t count)
	{
		fir_length = count;

		fir_coeffs.clear();
		for(size_t i = 0; i < count; i++) {
			fir_coeffs.push_back(L * firCoeffs[i]);
		}

		n = 1 + fir_length / L;

		// add padding
		size_t r = n * L - fir_length;
		for(size_t i = 0; i < r; i++) {
			fir_coeffs.push_back(0.0);
		}

		history0.resize(fir_length, 0.0);
		history1.resize(fir_length, 0.0);
		index = fir_length - 1;
	}

	void reset()
	{
		std::fill(history0.begin(), history0.end(), 0.0);
		std::fill(history1.begin(), history1.end(), 0.0);
		index = fir_length - 1;
	}

	void upsampleBlockMono(OutputType* output, const InputType* input, size_t sampleCount)
	{
		for(size_t s = 0; s < sampleCount; s++) {
			upsampleSingleMono(output, *input++);
			output += L;
		}
	}

	void upsampleBlockStereo(OutputType* output0, OutputType* output1, const InputType* interleaved, size_t sampleCount)
	{
		for(size_t s = 0; s < sampleCount; s ++) {
			upsampleSingleStereo(output0, output1, *interleaved, *(interleaved + 1));
			interleaved += 2;
			output0 += L;
			output1 += L;
		}
	}

	inline void upsampleSingleMono(OutputType* output, InputType input)
	{

#ifdef UPSAMPLER_TIME_FUNC
		static double renderTime = 0.0;
		FuncTimerQ funcTimer(&renderTime);
		constexpr size_t historyLength = 100;
		static MovingAverage<double, historyLength> movingAverage;
		static int64_t callCount = 0;

		const double mov_avg_renderTime = movingAverage.get(renderTime);

		constexpr int64_t every = 10000;
		if(++callCount % every == 0) {
			qDebug() << QStringLiteral("Avg Upsample time(last %1)=%2ns")
						.arg(historyLength)
						.arg(mov_avg_renderTime, 0, 'f', 2);
		}
#endif

		history0[index] = static_cast<OutputType>(input);
		size_t p = index;

		memset(output, 0.0, L * sizeof(OutputType));

		for(size_t j = 0; j < n * L; j+= L) {
			for(size_t k = 0; k < L; k++) {
				output[k] += fir_coeffs[j+k] * history0[p];
			}

			if(++p == fir_length) {
				p = 0;
			}
		}

		if(index-- == 0) {
			index = fir_length - 1;
		}
	}

	inline void upsampleSingleStereo(OutputType* output0, OutputType* output1, InputType input0, InputType input1)
	{
		history0[index] = static_cast<OutputType>(input0);
		history1[index] = static_cast<OutputType>(input1);

		size_t p = index;

		memset(output0, 0.0, L * sizeof(OutputType));
		memset(output1, 0.0, L * sizeof(OutputType));

		for(size_t j = 0; j < n * L; j+= L) {
			for(size_t k = 0; k < L; k++) {
				output0[k] += fir_coeffs[j+k] * history0[p];
				output1[k] += fir_coeffs[j+k] * history1[p];
			}

			if(++p == fir_length) {
				p = 0;
			}
		}

		if(index-- == 0) {
			index = fir_length - 1;
		}
	}

	size_t delayTime() const
	{
		return (fir_length - 1)  / 2;
	};
};

#endif // UPSAMPLER_H
