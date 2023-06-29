#ifndef SWEEPPARAMETERS_H
#define SWEEPPARAMETERS_H

#include <QDebug>

struct SweepParameters
{
	friend class ScopeWidget;
	double triggerTolerance{0.01};
	double triggerLevel{0.0};
	double slope{1.0};

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
        qDebug() << formatTimeDuration(newDuration_s);
		calcSweepAdvance();
	}

	void setDuration_ms(double newDuration_ms)
	{
		duration_ms = newDuration_ms;
		calcSweepAdvance();
	}

	double getSamplesPerSweep() const
	{
		return duration_ms * inputFrames_per_ms;
	}

    static QString formatTimeDuration(double duration_s)
    {
       // constexpr int m = 999;
        static QMap<int, QString> multiplierToPrefixMap
        {
            {-12, "ps"},
            {-9, "ns"},
            {-6, "us"},
            {-3, "ms"},
            {0, "s"},
            {3, "ks"},
            {6, "Ms"},
            {9, "Gs"}
        };

        int p = 3 * std::floor(std::log10(std::abs(duration_s)) / 3.0);
        return QStringLiteral("%1%2")
            .arg(static_cast<double>(duration_s / std::pow(10, p)), 0, 'f', 0)
            .arg(multiplierToPrefixMap.value(p));

    }


private:
	double duration_ms{10.0};
	int width_pixels{640};
	double inputFrames_per_ms{44.1};
	double sweepAdvance; // pixels per input frame
	double triggerMin{triggerLevel - triggerTolerance};
	double triggerMax{triggerLevel + triggerTolerance};

	void setWidthFrameRate(int width_pixels, double inputFrames_per_ms)
	{
		this->width_pixels = width_pixels;
		this->inputFrames_per_ms = inputFrames_per_ms;
		calcSweepAdvance();
	}

	void calcSweepAdvance()
	{
		sweepAdvance = width_pixels / inputFrames_per_ms / duration_ms;
	}
};

#endif // SWEEPPARAMETERS_H
