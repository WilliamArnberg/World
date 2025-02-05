#include "stdafx.h"
#include "ecs_World.h"

#include <utility>

#include "ComponentTypes.h"
#include "Jolt/Math/Mat44.h"
#include "Tools/PerfJuggler.h"
#include "System.h"
#include "Entity.h"
#include "QueryIterator.h"
#include "Collision/ColliderComponent.h"
#undef min

namespace ecs {



	World::World() : mySystems(std::make_unique<SystemManager>())
	{

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
		entity entity = GenerateID();


		Entity e(entity, this);
		Type emptyType;
		auto it = myArchetypeIndex.find(emptyType);
		Archetype* archetype;

		if (it == myArchetypeIndex.end())
		{
			myArchetypeIndex[emptyType];
			myArchetypeIndex[emptyType].SetID(myArchetypeIndex.size());
			myArchetypeIndex[emptyType].SetType(emptyType);
			myArchetypeIndex[emptyType].AddEntity(entity);

			archetype = &myArchetypeIndex[emptyType];
		}
		else
		{
			archetype = &it->second;
			archetype->AddEntity(entity);
		}

		//size_t newRow = archetype->components.empty() ? 0 : archetype->components[0].count;
		Record record;
		record.archetype = archetype;
		record.row = archetype->GetNumEntities() - 1;

		myEntityIndex.emplace(entity, record);

		return e;
	}

	bool World::DestroyEntity(ecs::entity id)
	{
		if (!myEntityIndex.contains(id)) return false;

		auto record = myEntityIndex.at(id);
		auto archetype = *record.archetype;
		size_t sourceRow = record.row;
		size_t lastRow = 0;

		if (!archetype.IsEmpty())
		{
			lastRow = archetype.GetNumEntities() - 1;
		}

		std::vector<ecs::entity>& entities = archetype.GetEntityList();

		if (sourceRow != lastRow)
		{
			ecs::entity entityToShuffle = archetype.GetEntity(lastRow);
			ecs::Record& shuffleRecord = myEntityIndex.at(entityToShuffle);
			shuffleRecord.row = sourceRow; //this is the shuffled entities new row.

			entities.at(shuffleRecord.row) = entities.at(lastRow);

			for (size_t i = 0; i < archetype.GetNumComponents(); i++)
			{
				void* sourceComponent = archetype.GetColumn(i)->GetComponent(lastRow);

				auto& typeData = archetype.GetColumn(i)->GetTypeInfo();

				void* targetComponent = archetype.GetColumn(i)->GetComponent(sourceRow);
				if (typeData.isTrivial)
				{
					std::memcpy(targetComponent, sourceComponent, archetype.GetColumn(i)->GetElementSize());
				}
				else if (typeData.move)
				{
					typeData.move(targetComponent, sourceComponent);
				}
				else if (typeData.copy)
				{
					typeData.copy(targetComponent, sourceComponent);
				}



				//std::memcpy(aArchetype.GetColumn(i)->GetComponent(sourceRow), aArchetype.GetColumn(i)->GetComponent(lastRow), aArchetype.GetColumn(i)->GetElementSize());
			}
		}

		myEntityIndex.erase(id);
		entities.pop_back();
		return true;
	}

	/*ecs::Entity ecs::World::Create(const char* aTag)
	{
		Entity e = Create();
		myTagToEntityIndex.emplace(aTag, e.GetID());
		return e;
	}*/

	//void World::TagEntity(const char* aTag, entity aEntity)
	//{
	//	myTagToEntityIndex.emplace(aTag, aEntity);
	//}

	//Entity World::GetEntity(const char* aTag)
	//{

	//	entity e = myTagToEntityIndex.at(aTag);
	//	return Entity(e, this);
	//}

	Entity World::GetEntity(entity id)
	{
		if (!myEntityIndex.contains(id))
		{
			return Entity(0xDEADBABE, this);
		}
		return Entity(id, this);
	}

	ecs::Archetype* ecs::World::GetArchetype(entity aEntity)
	{
		return myEntityIndex.at(aEntity).archetype;
	}

	void World::system(const char* aName, System&& aSystem, Pipeline aPipeline) const
	{
		mySystems->AddSystem(std::move(aSystem), aName, aPipeline);
	}


	void World::Clear()
	{
		myEntityIndex.clear();
		myArchetypeIndex.clear();
		myComponentIndex.clear();
		//myTagToEntityIndex.clear();

	}

	CleanUp World::ClearOnLoad()
	{
		std::vector<std::vector<ecs::entity>> entitiesToRemove;
		CleanUp cleanUp{};

		for (auto e : FilteredQuery<CCollider>(std::tuple<DontDestroyOnLoad>()))
		{
			auto col = e.GetComponent<CCollider>();
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
		myClearOnLoadArchetypeList.clear();
		myClearOnLoadIndex.clear();
		return cleanUp;
	}

	void World::Progress()
	{
		mySystems->Progress();
	}

	ecs::entity World::GenerateID()
	{
		ecs::entity id = myEntityIndexCounter++;
		if (id == 0xDEADBABE)
		{
			id = myEntityIndexCounter++;
		}
		return id;
	}

	void ecs::World::MoveEntityFromToArchetype(Archetype& aArchetype, entity aEntity, Archetype& aNewArchetype)
	{
		Record& record = myEntityIndex.at(aEntity);
		assert(record.archetype, "Archetype was null");
		aNewArchetype.AddEntity(aEntity); // the archetype count increases by 1
		size_t aNewRow = aNewArchetype.GetNumEntities() - 1;

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
				//assert(aNewArchetype.type[targetColumnIndex] == sourceID); //
				aNewArchetype.GetColumn(targetColumnIndex)->Resize(2);
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
			size_t elementSize = aArchetype.GetColumn(sourceColumnIndex)->GetElementSize();

			assert(aEntity == aArchetype.GetEntity(sourceRow) && aEntity == aNewArchetype.GetEntity(aNewRow), "Copying data to wrong entity.");
			//assert(aNewArchetype.GetComponentIDFromTypeList(targetColumnIndex) == aArchetype.GetComponentIDFromTypeList(sourceColumnIndex)); //Fixed: This doesn't work since adding tags typelists no longer are guaranteed to align.
			assert(aNewArchetype.GetColumn(targetColumnIndex)->GetElementSize() == aArchetype.GetColumn(sourceColumnIndex)->GetElementSize());

			aArchetype.GetColumn(sourceColumnIndex)->ChangeMemoryUsed(-1);
			aNewArchetype.GetColumn(targetColumnIndex)->ChangeMemoryUsed(1);

			void* sourceComponent = aArchetype.GetColumn(sourceColumnIndex)->GetComponent(sourceRow);
			auto typeData = aArchetype.GetColumn(sourceColumnIndex)->GetTypeInfo();
			void* targetComponent = aNewArchetype.GetColumn(targetColumnIndex)->GetComponent(aNewRow);
			if (typeData.isTrivial)
			{
				std::memcpy(targetComponent, sourceComponent, elementSize);
			}
			else if (typeData.move)
			{
				typeData.move(targetComponent, sourceComponent);
			}
			else if (typeData.copy)
			{
				typeData.copy(targetComponent, sourceComponent);
			}
		}

		record.archetype = &aNewArchetype;
		record.row = aNewRow;

		size_t lastRow = 0;
		if (!aArchetype.IsEmpty())
		{
			lastRow = aArchetype.GetNumEntities() - 1;
		}

		std::vector<ecs::entity>& entities = aArchetype.GetEntityList();
		
		if (sourceRow != lastRow)
		{
			ecs::entity entityToShuffle = aArchetype.GetEntity(lastRow);
			ecs::Record& shuffleRecord = myEntityIndex.at(entityToShuffle);
			shuffleRecord.row = sourceRow;
			entities.at(shuffleRecord.row) = entities.at(lastRow);

			for (size_t i = 0; i < aArchetype.GetNumComponents(); i++)
			{
				auto typeData = aArchetype.GetColumn(i)->GetTypeInfo();

				if (typeData.isTrivial)
				{
					std::memcpy(aArchetype.GetColumn(i)->GetComponent(sourceRow), aArchetype.GetColumn(i)->GetComponent(lastRow), aArchetype.GetColumn(i)->GetElementSize());
				}
				else if (typeData.move)
				{
					typeData.move(aArchetype.GetColumn(i)->GetComponent(sourceRow), aArchetype.GetColumn(i)->GetComponent(lastRow));
				}
				else if (typeData.copy)
				{
					typeData.copy(aArchetype.GetColumn(i)->GetComponent(sourceRow), aArchetype.GetColumn(i)->GetComponent(lastRow));
				}
			}

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
