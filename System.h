#pragma once

#include <functional>
#include <string>
#include <vector>
#include "WorldTimer.h"

namespace ecs {
	using System = std::function<void()>;

	enum class Pipeline
	{

		OnStart,	//This runs once when the game starts.
		OnLoad,		//Load data into ECS, keyboard and mouse input etc.
		PostLoad,	//Process Loaded data
		PreUpdate,	//Things that must happen before actual game logic,clean-up etc
		OnUpdate,	//Gameplay and systems
		OnValidate, //Validate what happened in the previous update for instance collision resolving
		OnRenderLoad,
		PostRenderLoad,
		PreRender,	//Begin Frame
		Render,		//Renders the scene
		UIRender,
		PostRender, //EndFrame Cleanup
		PreQuit, //CleanUp before shutting of systems
		OnQuit,
		DebugStart,
		DebugPreUpdate,
		DebugUpdate,
		DebugRender,
		DebugPostRender,
		Count
	};
	struct PairHash
	{
		template <typename T1, typename T2>
		std::size_t operator()(const std::pair<T1, T2>& pair) const
		{
			std::size_t hash1 = std::hash<T1>{}(pair.first);
			std::size_t hash2 = std::hash<T2>{}(pair.second);
			return hash1 ^ (hash2 << 1);
		}
	};
	class SystemManager
	{
	public:
		SystemManager();
		bool Progress();
		void AddSystem(const System&& aSystem, const char* aName, Pipeline aPipeline = Pipeline::OnUpdate);
		void RemoveSystem(const char* aName, Pipeline aPipeline);
		void Quit();
		float DeltaTime() const;
		float FixedTime() const;
		float TotalTime() const;
		int TickCount() const;
	private:
		void DebugOnStart();
		void OnStart();
		void OnLoad();
		void PostLoad();
		void PreUpdate();
		void OnUpdate();
		void OnValidate();
		void PreRender();
		void OnRenderLoad();
		void PostRenderLoad();
		void Render();
		void PostRender();
		void UIRender();
		void PreQuit();
		void OnQuit();
		void RemoveSystems();

		void DebugOnUpdate();
		void DebugPreUpdate();
		void DebugRender();
		void DebugPostRender();
		std::unordered_map <Pipeline, std::vector<std::pair<std::string, System>>> myPipelines;
		std::unordered_map<std::pair<std::string, Pipeline>, size_t, PairHash> mySystemIndex;
		std::vector<std::pair<std::string, ecs::Pipeline>> mySystemsToRemoveThisFrame;
		WorldTimer myTimer;
		bool myShouldQuit = false;
		bool myIsStarted = false;
#ifndef _RETAIL
		bool myDebugIsStarted = false;
#endif
	};

}



