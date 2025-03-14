#include "stdafx.h"
#include "ecs_World.h"

#include <utility>
#include <mutex>
#include "ComponentTypes.h"
#include "Jolt/Math/Mat44.h"
#include "Tools/PerfJuggler.h"
#include "System.h"
#include "Entity.h"
#include "QueryIterator.h"
#include "Collision/ColliderComponent.h"
#include "Collision/RagdollComponent.h"

#undef min

namespace ecs {



	World::World() : mySystems(std::make_unique<SystemManager>())
	{

		Type emptyType{};
		myArchetypeIndex[emptyType];
		myArchetypeIndex[emptyType].SetID(myArchetypeIndex.size());
		myArchetypeIndex[emptyType].SetType(emptyType);

	}

	World::~World()
	{
	}

	/*void SetBit(uint64_t& aValueToChange, uint64_t bit)
	{
	aValueToChange = aValueToChange | 1 << bit;
	}*/

	ecs::Entity ecs::World::Create()
	{
		const std::lock_guard<std::mutex> lock(myMutex);
		EntityID id = GenerateID();
		
		Entity e(id, this);
		Type emptyType{};
		Archetype& emptyArchetype = myArchetypeIndex.at(emptyType);
		emptyArchetype.AddEntity(id);

		myEntityIndex.emplace(id, Record{ &emptyArchetype,emptyArchetype.GetLastRow() });
		return e;
	}

	bool World::DestroyEntity(ecs::EntityID id)
	{
		if (!myEntityIndex.contains(id)) return false;

		auto record = myEntityIndex.at(id);
		Archetype* archetype = record.archetype;
		size_t sourceRow = record.row;
		size_t lastRow = 0;

		if (!archetype->IsEmpty())
		{
			lastRow = archetype->GetLastRow();
		}

		std::vector<ecs::EntityID>& entities = archetype->GetEntityList();

		if (sourceRow != lastRow)
		{
			ecs::EntityID entityToShuffle = archetype->GetEntity(lastRow);

			ecs::Record& shuffleRecord = myEntityIndex.at(entityToShuffle);
			shuffleRecord.row = sourceRow; //this is the shuffled entities new row.
			entities.at(shuffleRecord.row) = entities.at(lastRow);
			archetype->ShuffleEntity(lastRow, sourceRow);
		}
		InvalidateCachedQueryFromMove(archetype, nullptr);

		myEntityIndex.erase(id);
		entities.pop_back();
		return true;
	}


	Entity World::GetEntity(EntityID id)
	{
		if (!myEntityIndex.contains(id))
		{
			return Entity(ECS_ENTITY_NULL, this);
		}
		return Entity(id, this);
	}

	bool World::IsNull(EntityID id)
	{
		return !myEntityIndex.contains(id);
	}

	const ecs::Archetype* ecs::World::GetArchetype(EntityID aEntity) const
	{
		return myEntityIndex.at(aEntity).archetype;
	}

	float World::DeltaTime() const
	{
		return mySystems->DeltaTime();
	}

	float World::TotalTime() const
	{
		return mySystems->TotalTime();
	}

	float World::FixedTime() const
	{
		return mySystems->FixedTime();
	}

	int32_t World::TickCount() const
	{
		return mySystems->TickCount();
	}

	void World::InvalidateCachedQueryFromMove(Archetype* oldArchetype, Archetype* newArchetype)
	{
		if (oldArchetype)
		{

			auto oldArchetypeID = oldArchetype->GetID();
			if (myArchetypeToQueries.contains(oldArchetypeID))
			{
				for (const auto& queryHash : myArchetypeToQueries.at(oldArchetypeID))
				{
					myCachedQueries.erase(queryHash);
				}
				myArchetypeToQueries.erase(oldArchetypeID);
			}
		}

		if (newArchetype)
		{
			auto newArchetypeID = newArchetype->GetID();
			if (myArchetypeToQueries.contains(newArchetypeID))
			{
				for (const auto& queryHash : myArchetypeToQueries.at(newArchetypeID))
				{
					myCachedQueries.erase(queryHash);
				}
				myArchetypeToQueries.erase(newArchetypeID);
			}
		}
	}

	void World::system(const char* aName, System&& aSystem, Pipeline aPipeline) const
	{
		mySystems->AddSystem(std::move(aSystem), aName, aPipeline);
	}

	void World::RemoveSystem(const char* aName, Pipeline aPipeline) const
	{
		mySystems->RemoveSystem(aName,aPipeline);
	}


	void World::Clear()
	{
		myEntityIndex.clear();
		myArchetypeIndex.clear();
		myComponentIndex.clear();
		myClearOnLoadArchetypeList.clear();
		myClearOnLoadArchetypeIDList.clear();
		myClearOnLoadIndex.clear();
		Type emptyType{};
		myArchetypeIndex[emptyType];
		myArchetypeIndex[emptyType].SetID(myArchetypeIndex.size());
		myArchetypeIndex[emptyType].SetType(emptyType);
	}

	void World::SetDontDestroyOnLoad(ecs::EntityID aEntityID)
	{
		if (!HasComponent<DontDestroyOnLoad>(aEntityID))
		{
			AddComponent<DontDestroyOnLoad>(aEntityID);
		}

		if (HasComponent<Parent>(aEntityID))
		{
			ecs::EntityID parent = GetComponent<Parent>(aEntityID)->GetParent();
			if (parent != ECS_ENTITY_NULL)
			{
				SetDontDestroyOnLoad(parent);
			}
		}
	}
	CleanUp World::PrepareCleanupForLevelLoad()
	{
		myCachedQueries.clear();
		std::vector<std::vector<ecs::EntityID>> entitiesToRemove;
		CleanUp cleanUp{};
		
		for (auto e : FilteredQuery<CCollider>(std::tuple<DontDestroyOnLoad,RagdollTag>())) 
		{
			
			auto col = e.GetComponent<CCollider>();
			//Parent* parent = e.GetComponent<Parent>(); 
			cleanUp.colliderEntitiesToRemove.emplace_back(col->myBodyID, e.GetID());

		}

		for (auto& type : myClearOnLoadArchetypeList)
		{
			if (type == nullptr && !myArchetypeIndex.contains(*type)) continue;
			auto& archetype = myArchetypeIndex.at(*type);

			entitiesToRemove.push_back(archetype.GetEntityList());
			archetype.Reset();
			//myArchetypeIndex.erase(*type);

		}
		for (auto& el : entitiesToRemove)
		{
			for (auto e : el)
			{
				myEntityIndex.erase(e);
			}
		}
		myCachedQueries.clear();
		myArchetypeToQueries.clear();
		myClearOnLoadArchetypeList.clear();
		myClearOnLoadIndex.clear();
		return cleanUp;
	}

	bool World::Progress()
	{
		return mySystems->Progress();
	}

	void World::Quit()
	{
		mySystems->Quit();
	}

	ecs::EntityID World::GenerateID()
	{
		ecs::EntityID id = myNextEntity++;

		return id;
	}

	void ecs::World::MoveEntityFromToArchetype(Archetype& aArchetype, EntityID aEntity, Archetype& aNewArchetype)
	{
		InvalidateCachedQueryFromMove(&aArchetype, &aNewArchetype);
		Record& record = myEntityIndex.at(aEntity);
		assert(record.archetype, "Archetype was null");
		aNewArchetype.AddEntity(aEntity); // the archetype count increases by 1
		size_t aNewRow = aNewArchetype.GetLastRow();

		size_t sourceRow = record.row;
		//if moving from one archetype to another and the new archetype dont have enough memory double the size of the new allocation. 
		if (aNewArchetype.GetNumEntities() > aNewArchetype.GetMaxCount())
		{
			for (size_t i = 0; i < aArchetype.GetNumTypes(); i++)
			{
				ComponentID sourceID = aArchetype.GetComponentIDFromTypeList(i);

				ecs::ArchetypeMap& archetypeMap = myComponentIndex.at(sourceID);
				if (archetypeMap.at(aArchetype.GetID()).columnIndex == -1) continue;
				if (!archetypeMap.contains(aNewArchetype.GetID())) continue;

				size_t targetColumnIndex = archetypeMap.at(aNewArchetype.GetID()).columnIndex;
				if (targetColumnIndex == -1) continue; //Its a tag;
				aNewArchetype.GetColumn(targetColumnIndex)->Resize(aNewArchetype.GetColumn(targetColumnIndex)->GetCapacity() * 2);
			}
			aNewArchetype.SetMaxCount(aNewArchetype.GetMaxCount() * 2);
		};

		for (size_t i = 0; i < aArchetype.GetNumTypes(); i++)
		{
			ecs::ArchetypeMap& archetypeMap = myComponentIndex.at(aArchetype.GetComponentIDFromTypeList(i));

			if (archetypeMap.size() < 2) continue; //When removing components, map contains only 1 of the one to remove
			int sourceColumnIndex = archetypeMap.at(aArchetype.GetID()).columnIndex;
			int targetColumnIndex = -1;
			archetypeMap.contains(aNewArchetype.GetID()) ? targetColumnIndex = archetypeMap.at(aNewArchetype.GetID()).columnIndex : targetColumnIndex; //If we are removing a component and the new archetype doesnt have the component we set it to -1 and just continue

			if (sourceColumnIndex == -1 || targetColumnIndex == -1) continue; //Its a tag or the component shouldn't exist

			assert(aEntity == aArchetype.GetEntity(sourceRow) && aEntity == aNewArchetype.GetEntity(aNewRow), "Copying data to wrong entity.");
			assert(aNewArchetype.GetColumn(targetColumnIndex)->GetElementSize() == aArchetype.GetColumn(sourceColumnIndex)->GetElementSize());

			aArchetype.GetColumn(sourceColumnIndex)->ChangeMemoryUsed(-1);
			aNewArchetype.GetColumn(targetColumnIndex)->ChangeMemoryUsed(1);

			void* sourceComponent = aArchetype.GetColumn(sourceColumnIndex)->GetComponent(sourceRow);
			void* targetComponent = aNewArchetype.GetColumn(targetColumnIndex)->GetComponent(aNewRow);
			aNewArchetype.GetColumn(targetColumnIndex)->MoveOrCopyDataFromTo(sourceComponent, targetComponent);
		}

		record.archetype = &aNewArchetype;
		record.row = aNewRow;

		size_t lastRow = 0;
		if (!aArchetype.IsEmpty())
		{
			lastRow = aArchetype.GetLastRow();
		}

		std::vector<ecs::EntityID>& entities = aArchetype.GetEntityList();

		if (sourceRow != lastRow)
		{
			ecs::EntityID entityToShuffle = aArchetype.GetEntity(lastRow);
			ecs::Record& shuffleRecord = myEntityIndex.at(entityToShuffle);
			shuffleRecord.row = sourceRow;
			entities.at(shuffleRecord.row) = entities.at(lastRow);
			aArchetype.ShuffleEntity(lastRow, sourceRow);
		}

		entities.pop_back(); // the moved entity is guaranteed to be at the end at this point so just pop it, and decrease rowcount of the archetype.
		//}
	}

	std::ostream& ecs::operator<<(std::ostream& os, const Archetype& aArchetype)
	{
		os << "\n" << "***********************************" << "\n";
		os << "Archetype ID: " << aArchetype.GetID() << "\n";
		os << "Archetype Components: " << "\n";
		os << "Max Count: " << aArchetype.GetMaxCount();
		for (int i = 0; i < aArchetype.GetNumTypes(); i++)
		{
			os << aArchetype.GetComponentIDFromTypeList(i).name() << "\n";
			os << "ElementSize:" << aArchetype.components[i].GetElementSize() << "\n";
			os << "BufferSize:" << aArchetype.components[i].GetCapacity() << "\n";
		}

		os << "Archetype Entities: " << "\n";
		for (int i = 0; i < aArchetype.entities.size();)
		{
			os << aArchetype.entities[i];
		}

		os << "***********************************" << "\n";

		return os;
	}





}
