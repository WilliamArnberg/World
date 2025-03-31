#include "System.h"
#include "World.h"

#include "pix\pix3.h"

ecs::SystemManager::SystemManager()
{

}
//static int tick = 0;
//static float loctime = 0;
bool ecs::SystemManager::Progress()
{
	myTimer.Advance();
	if (!myIsStarted)
	{
		OnStart();
		myIsStarted = true;
	}
#ifndef _RETAIL
	if (!myDebugIsStarted)
	{
		DebugOnStart();
		myDebugIsStarted = true;
	}
#endif


		OnLoad();


		PostLoad();


#ifndef _RETAIL
	DebugPreUpdate();
	DebugOnUpdate();
#endif
	/*while (myTimer.ShouldRunFixed())
	{*/


		PreUpdate();

		OnUpdate();


		OnValidate();


		//myTimer.FixedTick();
	/*}*/
	//loctime += myTimer.GetDeltaTime();
	//myTimer.CalculateAlpha();


	OnRenderLoad();


	PostRenderLoad();


	PreRender();


	Render();


	UIRender();


#ifndef _RETAIL
	DebugRender();
	DebugPostRender();
#endif
	PostRender();


	RemoveSystems();



	Log::Impl::FinalPrint();

	if (myShouldQuit)
	{
		PreQuit();
		OnQuit();
		return false;
	}
	return true;

}

void ecs::SystemManager::AddSystem(const System&& aSystem, const char* aName, Pipeline aPipeline)
{
#if defined(_RETAIL)
	if (aPipeline == Pipeline::DebugUpdate || aPipeline == Pipeline::DebugRender) return;
#endif

	myPipelines[aPipeline].emplace_back(aName, aSystem);
	size_t index = myPipelines[aPipeline].size() - 1;
	mySystemIndex[{aName, aPipeline}] = index;
}

void ecs::SystemManager::RemoveSystem(const char* aName, Pipeline aPipeline)
{
	mySystemsToRemoveThisFrame.emplace_back(aName, aPipeline);


}

void ecs::SystemManager::Quit()
{
	myShouldQuit = true;
}

void ecs::SystemManager::DebugOnUpdate()
{
	PIXBeginEvent(0, "DebugOnUpdate");
	for (auto& [name, system] : myPipelines[Pipeline::DebugUpdate])
	{
		PIXBeginEvent(1, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::DebugOnStart()
{

	PIXBeginEvent(10, "DebugOnStart");
	for (auto& [name, system] : myPipelines[Pipeline::DebugStart])
	{
		PIXBeginEvent(11, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::UIRender()
{
	PIXBeginEvent(20, "UIRender");
	for (auto& [name, system] : myPipelines[Pipeline::UIRender])
	{
		PIXBeginEvent(21, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::PreQuit()
{
	PIXBeginEvent(30, "PreQuit");
	for (auto& [name, system] : myPipelines[Pipeline::PreQuit])
	{
		PIXBeginEvent(31, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::OnQuit()
{
	PIXBeginEvent(40, "OnQuit");
	for (auto& [name, system] : myPipelines[Pipeline::OnQuit])
	{
		PIXBeginEvent(41, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::RemoveSystems()
{
	for (auto& [name, pipeline] : mySystemsToRemoveThisFrame)
	{
		auto index = mySystemIndex[{name, pipeline}];
		auto& systemVec = myPipelines.at(pipeline);
		systemVec[index] = std::move(systemVec.back());
		systemVec.pop_back();
	}
	mySystemsToRemoveThisFrame.clear();
}

void ecs::SystemManager::OnStart()
{
	PIXBeginEvent(50, "OnStart");
	for (auto& [name, system] : myPipelines[Pipeline::OnStart])
	{
		PIXBeginEvent(51, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::OnLoad()
{
	PIXBeginEvent(60, "OnLoad");
	for (auto& [name, system] : myPipelines[Pipeline::OnLoad])
	{
		PIXBeginEvent(61, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::PostLoad()
{
	PIXBeginEvent(70, "PostLoad");
	for (auto& [name, system] : myPipelines[Pipeline::PostLoad])
	{
		PIXBeginEvent(71, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::DebugPreUpdate()
{
	PIXBeginEvent(80, "DebugPreUpdate");
	for (auto& [name, system] : myPipelines[Pipeline::DebugPreUpdate])
	{
		PIXBeginEvent(81, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::PreUpdate()
{
	PIXBeginEvent(90, "PreUpdate");
	for (auto& [name, system] : myPipelines[Pipeline::PreUpdate])
	{
		PIXBeginEvent(91, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::OnUpdate()
{
	PIXBeginEvent(100, "OnUpdate");
	for (auto& [name, system] : myPipelines[Pipeline::OnUpdate])
	{
		PIXBeginEvent(101, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::OnValidate()
{
	PIXBeginEvent(110, "OnValidate");
	for (auto& [name, system] : myPipelines[Pipeline::OnValidate])
	{
		PIXBeginEvent(111, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::PreRender()
{
	PIXBeginEvent(120, "PreRender");
	for (auto& [name, system] : myPipelines[Pipeline::PreRender])
	{
		PIXBeginEvent(121, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::OnRenderLoad()
{
	PIXBeginEvent(130, "OnRenderLoad");
	for (auto& [name, system] : myPipelines[Pipeline::OnRenderLoad])
	{
		PIXBeginEvent(131, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::PostRenderLoad()
{
	PIXBeginEvent(140, "PostRenderLoad");
	for (auto& [name, system] : myPipelines[Pipeline::PostRenderLoad])
	{
		PIXBeginEvent(141, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::Render()
{
	PIXBeginEvent(150, "Render");
	for (auto& [name, system] : myPipelines[Pipeline::Render])
	{
		PIXBeginEvent(151, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::DebugRender()
{
	PIXBeginEvent(160, "DebugRender");
	for (auto& [name, system] : myPipelines[Pipeline::DebugRender])
	{
		PIXBeginEvent(161, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::DebugPostRender()
{
	PIXBeginEvent(170, "DebugPostRender");
	for (auto& [name, system] : myPipelines[Pipeline::DebugPostRender])
	{
		PIXBeginEvent(171, name.c_str());
		system();
		PIXEndEvent();
	}
	PIXEndEvent();
}

void ecs::SystemManager::PostRender()
{
	PIXBeginEvent(180, "PostRender");
	for (auto& [name, system] : myPipelines[Pipeline::PostRender])
	{
		PIXBeginEvent(181, name.c_str());
		system();
		PIXEndEvent();
	}
		PIXEndEvent();
}
float ecs::SystemManager::DeltaTime() const
{
	return myTimer.GetDeltaTime();
}

float ecs::SystemManager::FixedTime() const
{
	return myTimer.GetFixedTime();
}

float ecs::SystemManager::TotalTime() const
{
	return myTimer.GetTotalTime();
}

int ecs::SystemManager::TickCount() const
{
	return myTimer.GetTickCount();
}