#include "System.h"
#include "World.h"


void ecs::SystemManager::Progress()
{
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
	PreUpdate();
	OnUpdate();
	OnValidate();
	PreRender();
	Render();
	UIRender();
#ifndef _RETAIL
	DebugRender();
	DebugPostRender();
#endif
	PostRender();
}

void ecs::SystemManager::AddSystem(const System&& aSystem, const char* aName, Pipeline aPipeline)
{
#if defined(_RETAIL)
	if (aPipeline == Pipeline::DebugUpdate || aPipeline == Pipeline::DebugRender) return;
#endif
	myPipelines[aPipeline].first = aName;
	myPipelines[aPipeline].second.emplace_back(aSystem);
	size_t index = myPipelines[aPipeline].second.size() - 1;
	mySystemIndex[{aName, aPipeline}] = index;
}

//void ecs::SystemManager::UpdateObservers(ObserverLists& /*aObserverList*/)
//{
//	//aObserverList;
//	/*for (const auto& lists : aObserverList)
//	{
//		for (auto& func : lists.second)
//		{
//			func()
//		}
//		
//
//	}*/
//}

void ecs::SystemManager::RemoveSystem(const char* aName, Pipeline aPipeline)
{
	auto index = mySystemIndex[{aName, aPipeline}];
	auto& systemVec = myPipelines.at(aPipeline).second;
	systemVec[index] = systemVec.back();
	systemVec.pop_back();
}

void ecs::SystemManager::DebugOnUpdate()
{
	for (auto& system : myPipelines[Pipeline::DebugUpdate].second)
	{
		system();
	}
}

void ecs::SystemManager::DebugOnStart()
{
	for (auto& system : myPipelines[Pipeline::DebugStart].second)
	{
		system();
	}
}

void ecs::SystemManager::UIRender()
{
	for (auto& system : myPipelines[Pipeline::UIRender].second)
	{
		system();
	}
}

void ecs::SystemManager::OnStart()
{
	for (auto& system : myPipelines[Pipeline::OnStart].second)
	{
		system();
	}
}

void ecs::SystemManager::OnLoad()
{
	for (auto& system : myPipelines[Pipeline::OnLoad].second)
	{
		system();
	}
}

void ecs::SystemManager::PostLoad()
{
	for (auto& system : myPipelines[Pipeline::PostLoad].second)
	{
		system();
	}
}

void ecs::SystemManager::DebugPreUpdate()
{
	for (auto& system : myPipelines[Pipeline::DebugPreUpdate].second)
	{
		system();
	}
}

void ecs::SystemManager::PreUpdate()
{
	for (auto& system : myPipelines[Pipeline::PreUpdate].second)
	{
		system();
	}
}

void ecs::SystemManager::OnUpdate()
{
	for (auto& system : myPipelines[Pipeline::OnUpdate].second)
	{
		system();
	}
}

void ecs::SystemManager::OnValidate()
{
	for (auto& system : myPipelines[Pipeline::OnValidate].second)
	{
		system();
	}
}

void ecs::SystemManager::PreRender()
{
	for (auto& system : myPipelines[Pipeline::PreRender].second)
	{
		system();
	}
}

void ecs::SystemManager::Render()
{
	for (auto& system : myPipelines[Pipeline::Render].second)
	{
		system();
	}
}

void ecs::SystemManager::DebugRender()
{
	for (auto& system : myPipelines[Pipeline::DebugRender].second)
	{
		system();
	}
}

void ecs::SystemManager::DebugPostRender()
{
	for (auto& system : myPipelines[Pipeline::DebugPostRender].second)
	{
		system();
	}
}

void ecs::SystemManager::PostRender()
{
	for (auto& system : myPipelines[Pipeline::PostRender].second)
	{
		system();
	}
}