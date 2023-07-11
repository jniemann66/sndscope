#ifndef SWEEPPARAMETERS_H
#define SWEEPPARAMETERS_H

#include <QDebug>
#include <cmath>

struct SweepParameters
{
	friend class ScopeWidget;
	double triggerTolerance{0.01};
	double triggerLevel{0.0};
	double slope{1.0};
	bool sweepUnused{false};
	bool triggerEnabled{true};
	int horizontalDivisions;
	int verticalDivisions;

public:
	double getDuration_ms() const
	{
		return duration_ms;
	}

	double getDuration() const
	{
		return duration_ms / 1000.0;
	}

	void setDuration(double newDuration_s)
	{
		duration_ms = 1000.0 * newDuration_s;
		calcSweepAdvance();
	}

	void setDuration_ms(double newDuration_ms)
	{
		duration_ms = newDuration_ms;
		calcSweepAdvance();
	}

	void setConnectDots(bool newConnectDots);

	double getSamplesPerSweep() const
	{
		return duration_ms * inputFrames_per_ms;
	}

	bool getConnectDots() const;

	double getUpsampleFactor() const;

	static QString formatMeasurementUnits(double duration_s, const QString& units, int precision = 2)
	{
		// constexpr int m = 999;
		static QMap<int, QString> prefixMap
		{
			{-30, "q"},
			{-27, "r"},
			{-24, "y"},
			{-21, "z"},
			{-18, "a"},
			{-15, "f"},
			{-12, "p"},
			{-9, "n"},
			{-6, "μ"},
			{-3, "m"},
			{0, ""},
			{3, "k"},
			{6, "M"},
			{9, "G"},
			{12, "T"},
			{15, "P"},
			{18, "E"},
			{21, "Z"},
			{24, "Y"},
			{27, "R"},
			{30, "Q"},

		};

		double m = 0.0;
		int p = 0;
		if(std::fpclassify(duration_s) != FP_ZERO) {
			p = 3 * std::floor(std::log10(std::abs(duration_s)) / 3.0);
			m =  static_cast<double>(duration_s / std::pow(10, p));
		}

		return QStringLiteral("%1 %2%3")
				.arg(m, 0, 'f', precision)
				.arg(prefixMap.value(p), units);
	}



	void setUpsampleFactor(double newUpsampleFactor);

private:
	double duration_ms{10.0};
	int width_pixels{640};
	double inputFrames_per_ms{44.1};
	double upsampleFactor{1.0};
	qreal sweepAdvance; // pixels per input frame
	double triggerMin{triggerLevel - triggerTolerance};
	double triggerMax{triggerLevel + triggerTolerance};
	bool connectDots{true};

	void setWidthFrameRate(int width_pixels, double inputFrames_per_ms)
	{
		this->width_pixels = width_pixels;
		this->inputFrames_per_ms = inputFrames_per_ms;
		calcSweepAdvance();
	}

	void calcSweepAdvance()
	{
		sweepAdvance = width_pixels / inputFrames_per_ms / duration_ms / upsampleFactor;
	}
};

inline bool SweepParameters::getConnectDots() const
{
	return connectDots;
}

inline double SweepParameters::getUpsampleFactor() const
{
	return upsampleFactor;
}

inline void SweepParameters::setUpsampleFactor(double newUpsampleFactor)
{
	upsampleFactor = newUpsampleFactor;
	calcSweepAdvance();
}

inline void SweepParameters::setConnectDots(bool newConnectDots)
{
	connectDots = newConnectDots;
}

#endif // SWEEPPARAMETERS_H
