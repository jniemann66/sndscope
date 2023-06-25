/*
* Copyright (C) 2016 - 2023 Judd Niemann - All Rights Reserved.
* You may use, distribute and modify this code under the
* terms of the GNU Lesser General Public License, version 2.1
*
* You should have received a copy of GNU Lesser General Public License v2.1
* with this file. If not, please refer to: https://github.com/jniemann66/ReSampler
*/

#ifndef _FUNCTIMER_H
#define _FUNCTIMER_H 

#include <iostream>
#include <iomanip>
#include <chrono>

// class FuncTimer : starts a high-resolution timer upon construction and
// upon destruction, accumulates time spent alive into total_duration

class FuncTimer {
	double* const total_duration;

	std::chrono::time_point<std::chrono::high_resolution_clock> beginTimer;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTimer;
public:

	explicit FuncTimer(double* total_duration) : total_duration(total_duration)
	{
		beginTimer = std::chrono::high_resolution_clock::now();
	}

	~FuncTimer()
	{
		endTimer = std::chrono::high_resolution_clock::now();
		if(total_duration != nullptr) {
			*total_duration += std::chrono::duration_cast<std::chrono::microseconds>(endTimer - beginTimer).count();
		}
	}
};


#endif // _FUNCTIMER_H
