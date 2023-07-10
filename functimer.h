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

#include <QElapsedTimer>

// class FuncTimer : starts a high-resolution timer upon construction and
// upon destruction, accumulates time spent alive into total_duration

template<typename D>
class FuncTimer
{
	double* const duration;

	std::chrono::time_point<std::chrono::high_resolution_clock> beginTimer;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTimer;
public:

	explicit FuncTimer(double* d) : duration(d)
	{
		beginTimer = std::chrono::high_resolution_clock::now();
//		beginTimer = std::chrono::system_clock::now();
	}

	~FuncTimer()
	{
		endTimer = std::chrono::high_resolution_clock::now();
		if(duration != nullptr) {
			*duration = std::chrono::duration_cast<D>(endTimer - beginTimer).count();
		}
	}
};

// Qt version
class FuncTimerQ
{
	QElapsedTimer timer;
	double* const duration;
public:
	explicit FuncTimerQ(double* d) : duration(d)
	{
		timer.start();
	}

	~FuncTimerQ()
	{
		*duration = timer.nsecsElapsed();
	}

};


#endif // _FUNCTIMER_H
