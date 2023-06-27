#ifndef SWEEPPARAMETERS_H
#define SWEEPPARAMETERS_H

struct SweepParameters
{
	friend class ScopeWidget;
	double threshold{0.01};
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

private:
	double duration_ms{10.0};
	int width_pixels{640};
	double inputFrames_per_ms{44.1};
	double sweepAdvance; // pixels per input frame

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
