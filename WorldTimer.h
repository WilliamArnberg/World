#pragma once

#include <Windows.h>
#include <profileapi.h>
namespace ecs
{
	class WorldTimer
	{
	public:
		WorldTimer();
		void Advance();
		inline float GetDeltaTime() const { return myTimings.realDeltaTime; }
		inline float GetTotalTime() const { return static_cast<float>(myTimings.totalTime); }
		inline float GetFixedTime() const { return static_cast<float>(myTimings.fixedTickRate); }
		inline int32_t GetTickCount() const{ return myTimings.tickCount;};
		bool ShouldRunFixed();
		void Pause();
		void FixedTick();
		void CalculateAlpha();
	private:
		double GetHighResTime();
		struct Timings
		{
			constexpr static int DEFAULT_TICK_RATE = 60;

			int32_t tickCount{ 0 };
			double lastTimestamp{ 0 };
			float fixedTickRate{ 1.0f / DEFAULT_TICK_RATE };
			float totalTime{ 0 };
			float realDeltaTime{ 0 };
			float accumulatedTime{ 0 };
			float interpolationAlpha{ 0 }; // For interpolation
		} myTimings;
		bool myShouldRunFixed{ true };
		static inline LARGE_INTEGER myLI{};
		__int64 CounterStart = 0;
		double myPCFreq = 0.0;

	};

}