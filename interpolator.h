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


#define INTERPOLATOR_TIME_FUNC

#ifdef INTERPOLATOR_TIME_FUNC
#include "movingaverage.h"
#include "functimer.h"
#endif

#include <vector>

#include <QDebug>

template <typename InputType, typename OutputType, int L>
class Interpolator
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
	static constexpr OutputType minPhase4_coefficients[] {
		0.001942459837306328,
		0.006466550747066193,
		0.01631812345562447,
		0.03400515391430036,
		0.0610198432728873,
		0.09645229368736537,
		0.135916236690805,
		0.1714843373916379,
		0.1931230249811224,
		0.1915775393594791,
		0.1619566335864067,
		0.1066352101583413,
		0.03611460183669894,
		-0.03307987518396896,
		-0.08320190704146363,
		-0.1013117043042409,
		-0.08424936680201797,
		-0.04037724336163016,
		0.01298198983685079,
		0.05575804487702125,
		0.07270007582428357,
		0.05922691539285639,
		0.0230346104563541,
		-0.0193685914293283,
		-0.0497961341526512,
		-0.05614708492817144,
		-0.03738851682256372,
		-0.003555190611602031,
		0.02918378039426254,
		0.04607844813356817,
		0.04045537865296101,
		0.01641687414384878,
		-0.01358102847423187,
		-0.03498829344992581,
		-0.03816271995507962,
		-0.02274188799883353,
		0.002646615726939195,
		0.02502351086291638,
		0.03361787660804501,
		0.02504795629750439,
		0.004613021689344724,
		-0.01678859048817612,
		-0.02839428849345416,
		-0.02496966306872431,
		-0.00915434483408965,
		0.01029100514121928,
		0.02326292167680339,
		0.02353682520670561,
		0.01174261408266126,
		-0.005332755251769948,
		-0.01857807697330734,
		-0.02139062932158502,
		-0.01295417212761777,
		0.001663643915142218,
		0.01448098430568106,
		0.0189311140261343,
		0.01321276825333682,
		0.0009554537675983014,
		-0.01100352449577649,
		-0.01640470012483835,
		-0.01282769246714429,
		-0.002736109333515083,
		0.00812252928656013,
		0.0139616194604492,
		0.01202481869526155,
		0.003858101416460902,
		-0.005787439991045355,
		-0.0116903339647864,
		-0.01097082333856446,
		-0.00447241806217718,
		0.003937798669246824,
		0.009640271302589997,
		0.009786821166327748,
		0.004703074628050162,
		-0.002509080419894071,
		-0.007833106017250822,
		-0.008561080325978677,
		-0.00465308494862259,
		0.001437756421649582,
		0.006273084655596417,
		0.00735661873567367,
		0.004406938481293616,
		-0.0006631880867268608,
		-0.004952739103937303,
		-0.006218078047846131,
		-0.004032793988411068,
		0.000130739866383867,
		0.003856766904129437,
		0.005173746748095328,
		0.003585780190330158,
		0.0002127501466720813,
		-0.002958855559271332,
		-0.004231533729455875,
		-0.003093417537169034,
		-0.0003882651018708267,
		0.002268917186775902,
		0.00344345677676334,
		0.002644678976185437,
		0.0005023177415877072,
		-0.001685600804682399,
		-0.002728193263942232,
		-0.002180467243756826,
		-0.0005115239020427131,
		0.00124758677571306,
		0.002128241848150291,
		0.001748492718563105,
		0.0004590664445129878,
		-0.0009297420879070221,
		-0.001644799726232371,
		-0.001372599581963624,
		-0.0003800993935805718,
		0.0007004233410324557,
		0.001262175019712139,
		0.001058413762135826,
		0.0002957038863593037,
		-0.0005342468357634072,
		-0.0009624257562023159,
		-0.0008016873900218317,
		-0.0002168199088837459,
		0.0004117081895406633,
		0.0007281339817820408,
		0.0005967235832692842,
		0.0001495670146938092,
		-0.0003203704852411982,
		-0.000546901875442695,
		-0.0004352399544501837,
		-9.453935240866534e-05,
		0.0002513864753335358,
		0.0004064278975972611,
		0.0003096435718671535,
		5.150208316990324e-05,
		-0.0001979998144356702,
		-0.0002974811634693959,
		-0.0002124910965349982,
		-2.024349313849857e-05,
		0.0001541140439704594,
		0.0002130802883578323,
		0.0001412373801740035,
		-1.653555895420196e-07,
		-0.0001191504392746235,
		-0.0001503373746984491,
		-8.922979860134165e-05,
		1.369557400999838e-05,
		9.142857488974345e-05,
		0.0001029273448220519,
		5.211485442021435e-05,
		-2.210238270544472e-05,
		-6.932267289766405e-05,
		-6.680004387749696e-05,
		-2.443519483800983e-05,
		2.46152974749597e-05,
		4.947416207185901e-05,
		3.99719693287932e-05,
		8.18652925045285e-06,
		-2.342557013240194e-05,
		-3.550685609487038e-05,
		-2.19334316245985e-05,
		5.015532035366642e-06,
		2.123460311912863e-05,
		2.205418652993056e-05,
		6.531053989792179e-06,
		-1.102676888504453e-05,
		-1.731574608270729e-05,
		-8.881588796751019e-06,
		5.555229167860656e-06,
		1.268252859091385e-05,
		5.935848987120287e-06,
		-6.151647641804148e-06,
		-6.620991960188782e-06,
		5.693367619837107e-06,
		-4.119187397212893e-06,
		-4.205037198105454e-07,
		1.140783880992241e-06
	};

	// FIR Filter
	size_t fir_length;
	std::vector<OutputType> fir_coeffs;
	std::vector<OutputType> history0;
	std::vector<OutputType> history1;

	size_t index;
	size_t n;

public:
	Interpolator()
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

#ifdef INTERPOLATOR_TIME_FUNC
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

#endif // INTERPOLATOR_H
