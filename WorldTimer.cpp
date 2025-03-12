#include "stdafx.h"
#include "WorldTimer.h"

namespace ecs
{
	WorldTimer::WorldTimer()
	{
		QueryPerformanceFrequency(&myLI);
		CounterStart = myLI.QuadPart;

		LARGE_INTEGER li;
		if (!QueryPerformanceFrequency(&li))
			std::cout << "QueryPerformanceFrequency failed!\n";

		myPCFreq = double(li.QuadPart);
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
		myTimings.lastTimestamp = GetHighResTime();
	}
	double WorldTimer::GetHighResTime()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);  // Get the current counter value

		double seconds = double(li.QuadPart - CounterStart) / myPCFreq;

		return seconds;

	}

	void WorldTimer::Advance()
	{
		double currentTime = GetHighResTime();
		double frameTime = currentTime - myTimings.lastTimestamp;

		if (frameTime > 0.25)
			frameTime = 0.25;

		myTimings.lastTimestamp = currentTime;
		myTimings.realDeltaTime = static_cast<float>(frameTime);
		myTimings.accumulatedTime += static_cast<float>(frameTime);
		myTimings.totalTime += static_cast<float>(frameTime);


	}


	bool WorldTimer::ShouldRunFixed()
	{
		if (myTimings.accumulatedTime >= myTimings.fixedTickRate) return true;

		return false;
	}

	void WorldTimer::Pause()
	{

	}

	void WorldTimer::FixedTick()
	{
		myTimings.accumulatedTime -= static_cast<float>(myTimings.fixedTickRate);
		myTimings.tickCount++;
	}

	void WorldTimer::CalculateAlpha()
	{
		myTimings.interpolationAlpha = static_cast<float>(myTimings.accumulatedTime / myTimings.realDeltaTime);
	}

}