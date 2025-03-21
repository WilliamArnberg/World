#pragma once
#include <unordered_map>
#include "Ecs_Aliases.h"
namespace ecs
{
	class World;
	class Stage : public World
	{
	public:
		friend World;
		Stage(World* aWorld);
		~Stage();
		
		Entity CreateEntity();
		void Merge();
		void ResetStage();
	private:
		World* myWorld;
	};

}