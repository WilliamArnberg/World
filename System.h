#pragma once

#include <functional>
#include <string>

namespace ecs {
	using System = std::function<void()>;

	enum class Pipeline
	{

		OnLoad,		//Load data into ECS, keyboard and mouse input etc.
		PostLoad,	//Process Loaded data
		PreUpdate,	//Things that must happen before actual game logic,clean-up etc
		OnUpdate,	//Gameplay and systems
		OnValidate, //Validate what happened in the previous update for instance collision resolving
		OnStart,	//This runs once when the game starts.
		PreRender,	//Begin Frame
		Render,		//Renders the scene
		UIRender,
		PostRender, //EndFrame Cleanup

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
		void PostRender();
		void Progress();
		void AddSystem(const System&& aSystem, const char* aName, Pipeline aPipeline = Pipeline::OnUpdate);
		void RemoveSystem(const char* aName, Pipeline aPipeline);

	private:
		void DebugOnStart();
		void OnStart();
		void OnLoad();
		void PostLoad();
		void PreUpdate();
		void OnUpdate();
		void OnValidate();
		void PreRender();
		void Render();
		void UIRender();

		void DebugOnUpdate();
		void DebugPreUpdate();
		void DebugRender();
		void DebugPostRender();

		std::unordered_map <Pipeline, std::pair<std::string, std::vector<System>>> myPipelines;
		std::unordered_map<std::pair<std::string,Pipeline>, size_t,PairHash> mySystemIndex;

		bool myIsStarted = false;
#ifndef _RETAIL
		bool myDebugIsStarted = false;
#endif
	};

}



