#include "stdafx.h"
#include "Stage.h"
#include "ecs_World.h"

namespace ecs
{
	ecs::Stage::Stage(World* aWorld) : myWorld(aWorld)
	{
	}
	ecs::Stage::~Stage()
	{
	}
	Entity ecs::Stage::CreateEntity()
	{
		auto nextEntity = myWorld->GenerateID();
		return Create(nextEntity);

	}

	void ecs::Stage::Merge()
	{
		for (auto& [type, sourceArchetype] : myArchetypeIndex)
		{
			if (myWorld->myArchetypeIndex.contains(type))
			{
				auto& targetArchetype = myWorld->myArchetypeIndex.at(type);
				
				if(targetArchetype.HasComponent(GetComponentID<DontDestroyOnLoad>())) continue;
				auto& list = sourceArchetype.GetEntityList();
				for(auto e : list)
				{

					myWorld->myEntityIndex.emplace(e,Record(&targetArchetype,myEntityIndex.at(e).row));
				}
				targetArchetype.Reset(sourceArchetype);
			}
			else
			{
				
				myWorld->myArchetypeIndex.emplace(type,Archetype());
				auto& newSource = myWorld->myArchetypeIndex.at(type);
				newSource = std::move(sourceArchetype);
				newSource.SetID(myWorld->GenerateArchetypeID());
				auto& list = newSource.GetEntityList();

				for(const auto& comp : type)
				{
						auto& am = myComponentIndex.at(comp).at(sourceArchetype.GetID());
						ArchetypeRecord ar(&newSource,am.columnIndex);
						myWorld->myComponentIndex[comp].emplace(newSource.GetID(),ar);
				}

				for(auto e : list)
				{
					myWorld->myEntityIndex.emplace(e,Record(&newSource,myEntityIndex.at(e).row));
				}
				
				//myWorld->myComponentIndex.
			}
		}
		

	}

	void Stage::ResetStage()
	{
		Clear();
	}

}
