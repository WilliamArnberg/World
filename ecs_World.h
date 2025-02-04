#pragma once
#include <cassert>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <memory>
#include <span>
#include <stack>
#include <mutex>
#include <string>
#include <vector>
#include <cstddef>

#include "ComponentTypes.h"
#include "System.h"
#include "Archetype.h"
#include "Ecs_Aliases.h"
#include "CleanUpContainer.h"
#include <variant>
#include <pix/pix3.h>
namespace ecs
{
	class Entity;
	class SystemManager;
	class QueryIterator;
	template<typename T>
	inline ComponentID GetComponentID();

#undef max
	constexpr int MAX_ENTITIES = 0xFFFFF;
	class World
	{
	public:
		friend Archetype;
		friend QueryIterator;
		World();
		~World();

		Entity Create();

		bool DestroyEntity(ecs::entity id);

		Entity GetEntity(entity id);
		

		template<typename... args>
		Archetype* GetArchetype();

		Archetype* GetArchetype(entity id);

		template<typename T> 
		bool HasComponent(entity e) const;

		template<typename T>
		T* GetComponent(entity e);

		template<typename... args>
		QueryIterator Query();

		template<typename... Components, typename... Filter>
		inline QueryIterator FilteredQuery(std::tuple<Filter...> filters);

		template<typename T>
		Entity TQuery();

		template<typename T>
		T* AddComponent(entity e);

		template<typename T>
		void RemoveComponent(entity e);

		template<typename T, typename...args>
		void Set(entity aEntity, args&&... aArgumentList);

		void system(const char* aName, System&& aSystem, Pipeline aPipeline = Pipeline::OnUpdate) const;

		template<typename T, typename Func>
		void Observe(entity aEntity, Func&& aFunc, ObserverType aType);

			template<typename T>
		void EnableComponent(ecs::entity aEntityID,bool isEnabled);

		//This clears the whole ecs level
		void Clear();
		CleanUp ClearOnLoad();
		void Progress();

	private:
		ecs::entity GenerateID();

		template<typename T>
		Archetype& AddArchetype(Archetype& aArchetypeSource);

		void MoveEntityFromToArchetype(Archetype& aArchetype, entity aEntity, Archetype& aNewArchetype);

		template<typename T>
		void InvokeObserverCallbacks(entity aEntity, ObserverType aType);

		template<typename T>
		ComponentTypeInfo RegisterComponent();

		//archetype Data
		std::mutex myMutex;
		uint64_t myEntityIndexCounter = 1;
		std::unordered_map<ComponentID, ArchetypeMap> myComponentIndex; // Used to lookup components in archetypes
		std::unordered_map<Type, Archetype, TypeHash, TypeEqual> myArchetypeIndex; // Find an archetype by its list of component ids
		std::unordered_map<entity, Record> myEntityIndex;		// Find the archetype for an entity

		//std::unordered_map<std::string, entity> myTagToEntityIndex; //Find entity by tag if it has one.

		std::unordered_map<ArchetypeID, size_t> myClearOnLoadIndex;
		std::vector<const Type*> myClearOnLoadArchetypeList;

		ObserverMap myObserverIndex;
		std::unique_ptr<SystemManager> mySystems;
	};

	template<typename T>
	void World::InvokeObserverCallbacks(entity aEntity, ObserverType aType)
	{
		ObserverRecord& observerRecord = myObserverIndex.at(GetComponentID<T>());
		ObserverLists& observerLists = observerRecord.at(aEntity);
		ObserverList& list = observerLists.at(aType);
		for (std::function<void()>& func : list)
		{
			func();
		}
	}

	template<typename T>
	inline ComponentTypeInfo World::RegisterComponent()
	{
		ComponentTypeInfo typeInfo;

		typeInfo.size = sizeof(T);
		typeInfo.alignment = alignof(T);
		typeInfo.typeID = typeid(T);
		// Default constructor
		typeInfo.construct = std::is_default_constructible_v<T> ?
			[](void* dest) { new (dest) T(); } : nullptr;

		// Copy constructor
		typeInfo.copy = std::is_copy_constructible_v<T> ?
			[](void* dest, const void* src) { new (dest) T(*reinterpret_cast<const T*>(src)); } : nullptr;

		// Move constructor
		typeInfo.move = std::is_move_constructible_v<T> ?
			[](void* dest, void* src) { new (dest) T(std::move(*reinterpret_cast<T*>(src))); } : nullptr;

		// Destructor
		typeInfo.destruct = std::is_destructible_v<T> ?
			[](void* obj) { reinterpret_cast<T*>(obj)->~T(); } : nullptr;

		//Trivial copyable check
		typeInfo.isTrivial = std::is_trivially_copyable_v<T>;

		return typeInfo;
	}

	template <typename ... args>
	Archetype* World::GetArchetype()
	{
		Type componentTypes = { std::type_index(typeid(args))... };
		std::sort(componentTypes.begin(), componentTypes.end());
		if (myArchetypeIndex.contains(componentTypes) == 0) { return nullptr; }

		return &myArchetypeIndex[componentTypes];
	}

	template<typename T>
	inline bool World::HasComponent(entity e) const
	{
		assert(myEntityIndex.contains(e), "I CANT BELIEVE YOU'VE DONE THIS.");

		return myEntityIndex.at(e).archetype->Contains(std::tuple<T>());
	}

	template <typename T>
	T* World::GetComponent(entity e)
	{
		std::lock_guard<std::mutex> lock(myMutex); //There are no guarantees for this making it thread-safe since if immediately after this gets unnlocked someones adds a component and the data moves the pointer will be invalid no matter what.
		if (std::is_empty<T>()) return nullptr; //You cannot fetch tags
		//if (!myEntityIndex.contains(e)) return nullptr;
		Record& record = myEntityIndex.at(e);
		if (!record.archetype->HasComponent(GetComponentID<T>())) return nullptr;
		ArchetypeMap& archetypeMap = myComponentIndex[GetComponentID<T>()];
		if (archetypeMap.count(record.archetype->GetID()) == 0) return nullptr;

		ArchetypeRecord& archetypeRecord = archetypeMap[record.archetype->GetID()];

		assert(record.archetype->GetID() == archetypeRecord.archetype->GetID());
		assert(record.archetype->GetEntityList().at(record.row) == e);


		T* component = static_cast<T*>(record.archetype->GetColumn(archetypeRecord.columnIndex)->GetComponent(record.row));
		return component;
	}

	template <typename ... args>
	QueryIterator World::Query()
	{

		std::lock_guard<std::mutex> lock(myMutex);
		std::vector<Archetype*> archetypeArray;
		Type types;
		types = { std::type_index(typeid(args))... };
		std::sort(types.begin(), types.end());
		if (!myComponentIndex.contains(types[0]))
		{
			return QueryIterator();
		}
		auto& archetypeMap = myComponentIndex.at(types[0]);
		for (const auto& record : archetypeMap)
		{
			size_t found = 1;

			for (int i = 1; i < types.size(); i++)
			{
				ComponentID type = types[i];
				if (record.second.archetype->HasComponent(type))
				{
					found++;
				}
				if (found < 2)
				{
					break;
				}
			}

			if (found == types.size() && 0 < record.second.archetype->GetNumEntities())
			{

				archetypeArray.push_back(record.second.archetype);
			}
		}

		if (!archetypeArray.empty())
		{
			return QueryIterator(this, archetypeArray);
		}
		return QueryIterator();
	}

	template<typename ...Components, typename ...Filter>
	inline QueryIterator World::FilteredQuery(std::tuple<Filter...> filters)
	{
		std::lock_guard<std::mutex> lock(myMutex);
		std::vector<Archetype*> archetypeArray;

		Type types;
		types = { std::type_index(typeid(Components))... };
		std::sort(types.begin(), types.end());
		if (!myComponentIndex.contains(types[0]))
		{
			return QueryIterator();
		}
		auto& archetypeMap = myComponentIndex.at(types[0]);
		for (const auto& record : archetypeMap)
		{
			if (record.second.archetype->Contains(filters)) continue;
			size_t found = 1;

			for (int i = 1; i < types.size(); i++)
			{

				ComponentID type = types[i];

				if (record.second.archetype->HasComponent(type))
				{
					found++;
				}
				if (found < 2)
				{
					break;
				}
			}
			if (found == types.size() && 0 < record.second.archetype->GetNumEntities())
			{

				archetypeArray.push_back(record.second.archetype);
			}
		}

		if (!archetypeArray.empty())
		{
			return QueryIterator(this, archetypeArray);
		}
		return QueryIterator();
	}

	template<typename T>
	inline Entity World::TQuery()
	{
		ComponentID id = GetComponentID<T>();
		if (!myComponentIndex.contains(id)) return Entity();

		const auto& am = myComponentIndex.at(id);
		for (const auto& record : am)
		{
			if (!record.second.archetype->IsEmpty())
			{
				ecs::entity entity = record.second.archetype->GetEntityList().at(0);
				return Entity(entity, this);
			};


		}

		return Entity();

	}

	template <typename T>
	T* World::AddComponent(entity e)
	{

		Record& record = myEntityIndex.at(e);

		T* component = GetComponent<T>(e);
		if (component) return component;

		ComponentID componentID = GetComponentID<T>();
		Archetype& archetype = *record.archetype;
		std::lock_guard<std::mutex> lock(myMutex);
		Archetype& nextArchetype = AddArchetype<T>(archetype);
		ArchetypeID nextArchetypeID = nextArchetype.GetID();

		assert(archetype.GetID() != nextArchetypeID, "Somehow moving to same archetype");
		size_t maxCount = nextArchetype.GetMaxCount();
		MoveEntityFromToArchetype(archetype, e, nextArchetype);

		if (!nextArchetype.HasComponent(GetComponentID<DontDestroyOnLoad>()) && !myClearOnLoadIndex.contains(nextArchetypeID))
		{
			myClearOnLoadArchetypeList.emplace_back(&nextArchetype.GetType());
			size_t index = myClearOnLoadArchetypeList.size();
			myClearOnLoadIndex.emplace(nextArchetypeID, index);
		}
		if (std::is_empty<T>())return nullptr;

		if (maxCount != nextArchetype.GetMaxCount()) //Checking this here because nextArchetype maxCount might change inside MoveEntityFromToArchetype
		{
			ArchetypeMap& archetypeMap = myComponentIndex.at(componentID);
			size_t columnID = archetypeMap.at(nextArchetypeID).columnIndex;
			nextArchetype.GetColumn(columnID)->Resize(2);
		}
		ArchetypeMap& am = myComponentIndex.at(componentID);
		ArchetypeRecord& archetypeRecord = am.at(nextArchetypeID);

		assert(nextArchetype.GetColumn(archetypeRecord.columnIndex)->GetTypeInfo().typeID == typeid(T),"This component is not the right type, imminent pagefault.");

		void* targetComponent = nextArchetype.GetColumn(archetypeRecord.columnIndex)->GetComponent(record.row);
		nextArchetype.GetColumn(archetypeRecord.columnIndex)->ChangeMemoryUsed(1);
		T* newComponent = static_cast<T*>(new (targetComponent) T());

		return newComponent;
	}

	template <typename T>
	void World::RemoveComponent(entity e)
	{
		auto& record = myEntityIndex.at(e);
		if (!record.archetype || !record.archetype->Contains(std::tuple<T>())) return;
		
		ArchetypeEdge& edges = record.archetype->GetOrAddEdge(GetComponentID<T>());
		if (edges.removeArchetypes)
		{
			MoveEntityFromToArchetype(*record.archetype, e, *edges.removeArchetypes);
		}
		else
		{
			
			Type newType = record.archetype->GetType();
			int found = 0;
			for (ComponentID& t : newType)
			{
				if (t == GetComponentID<T>()) break;

				found++;
			}

			newType[found] = newType.back();
			newType.pop_back();

			std::sort(newType.begin(), newType.end());
			if (myArchetypeIndex.contains(newType))
			{
				auto& newArchetype = myArchetypeIndex.at(newType);
				MoveEntityFromToArchetype(*record.archetype, e, newArchetype);
			}
			else
			{
				auto& newArchetype = myArchetypeIndex[newType];

				ComponentID componentID = GetComponentID<T>();


				newArchetype.SetID(myArchetypeIndex.size());
				newArchetype.SetType(newType);

				size_t numComponents{ 0 };
				bool isTag = false;
				int columnIndex = 0;
				ArchetypeID sourceArchetype  = record.archetype->GetID();
				for (int i = 0; i < newType.size(); ++i)
				{
					newArchetype.AddComponentIDToTypeSet(newType[i]);
					ArchetypeMap& am = myComponentIndex[newType[i]];
					am[sourceArchetype].columnIndex < 0 ? isTag = true : isTag = false;

					if (isTag)
					{
						am[newArchetype.GetID()].columnIndex = -1;
						am[newArchetype.GetID()].archetype = &myArchetypeIndex[newType];
						isTag = false;
					}
					else
					{
						am[newArchetype.GetID()].columnIndex = columnIndex;
						am[newArchetype.GetID()].archetype = &myArchetypeIndex[newType];
						numComponents++;
						columnIndex++;
					}

				}


				/*for (int columnIndex = 0; columnIndex < newType.size(); ++columnIndex)
				{
					ComponentID type = newType[columnIndex];
					newArchetype.AddComponentIDToTypeSet(type);
					ArchetypeMap& am = myComponentIndex[type];
					am[newArchetype.GetID()].column = columnIndex;
					am[newArchetype.GetID()].archetype = &myArchetypeIndex[newType];
				}*/

				//Copying over component structure from old archetype to new archetype, no data is copied at this point.
				//Only copying meta data for component structure 
				size_t maxCount = 2;
				newArchetype.ReserveComponentsSize(record.archetype->GetNumComponents() - 1);
				newArchetype.SetMaxCount(maxCount);

				for (int i = 0; i < newArchetype.GetComponentCapacity(); i++)
				{
					newArchetype.AddEmptyComp();
					
				}

				for (int i = 0; i < record.archetype->GetNumTypes(); i++)
				{
					if (record.archetype->GetType()[i] == GetComponentID<T>())continue;
					auto& archetypeMap = myComponentIndex.at(record.archetype->GetType()[i]);

					int sourceColumnIndex = archetypeMap.at(record.archetype->GetID()).columnIndex;
					int targetColumnIndex = archetypeMap.at(newArchetype.GetID()).columnIndex;
					if (sourceColumnIndex == -1 || targetColumnIndex == -1) continue; //Its a tag

					newArchetype.GetColumn(targetColumnIndex)->AssignTypeInfo(record.archetype->GetColumn(sourceColumnIndex)->GetTypeInfo());
					
					size_t elementSize = newArchetype.GetColumn(targetColumnIndex)->GetElementSize();
					newArchetype.GetColumn(targetColumnIndex)->Reset(new std::byte[elementSize * maxCount]);
					newArchetype.GetColumn(targetColumnIndex)->SetCapacity(maxCount * elementSize);
				}

				ArchetypeEdge& edge = record.archetype->GetEdge(componentID);
				edge.removeArchetypes = &myArchetypeIndex[newType];
				
				newArchetype.GetOrAddEdge(componentID).removeArchetypes = nullptr;
				newArchetype.GetOrAddEdge(componentID).addArchetypes = record.archetype;

			}

		}



	}

	template<typename T, typename ...args>
	inline void World::Set(entity aEntity, args&&... aArgumentList)
	{
		ArchetypeMap& am = myComponentIndex.at(GetComponentID<T>());
		const auto index = myEntityIndex.at(aEntity).row;
		const auto archetype = myEntityIndex.at(aEntity).archetype;
		const auto archetypeID = myEntityIndex.at(aEntity).archetype->GetID();;
		const auto columnIndex = am.at(archetypeID).columnIndex;
		const auto& column = archetype->components.at(columnIndex);

		InvokeObserverCallbacks<T>(aEntity, ecs::ObserverType::OnSet);
		T* t = static_cast<T*>(column[index]);
		*t = T(std::forward<args>(aArgumentList)...);
	}

	template <typename T, typename Func>
	void World::Observe(entity aEntity, Func&& aFunc, ObserverType aType)
	{
		ObserverRecord& observerRecord = myObserverIndex[GetComponentID<T>()];
		ObserverLists& observerLists = observerRecord[aEntity];
		observerLists[aType].emplace_back(std::forward<Func>(aFunc));
	}

	template<typename T>
	inline void World::EnableComponent(ecs::entity aEntityID, bool isEnabled)
	{
	}

	

	
	template <typename T>
	Archetype& World::AddArchetype(Archetype& aArchetypeSource)
	{
		ComponentID componentID = GetComponentID<T>();


		Type newType = aArchetypeSource.GetType();
		newType.emplace_back(componentID);
		std::sort(newType.begin(), newType.end());

		auto it = myArchetypeIndex.find(newType);
		if (it != myArchetypeIndex.end())
		{
			return it->second;
		}

		myArchetypeIndex[newType] = Archetype();
		myArchetypeIndex[newType].SetID(myArchetypeIndex.size());
		myArchetypeIndex[newType].SetType(newType);

		auto& newArchetype = myArchetypeIndex[newType];
		size_t numComponents{ 0 };
		bool isTag = false;
		int columnIndex = 0;
		for (int i = 0; i < newType.size(); ++i)
		{

			newArchetype.AddComponentIDToTypeSet(newType[i]);
			ArchetypeMap& am = myComponentIndex[newType[i]];

			if (aArchetypeSource.HasComponent(newType[i]))
			{
				isTag = am[aArchetypeSource.GetID()].columnIndex < 0 ? true : false;
			}
			if (isTag || (newType[i] == typeid(T) && std::is_empty<T>()))
			{
				am[newArchetype.GetID()].columnIndex = -1;
				am[newArchetype.GetID()].archetype = &myArchetypeIndex[newType];
				isTag = false;
			}
			else
			{
				am[newArchetype.GetID()].columnIndex = columnIndex;
				am[newArchetype.GetID()].archetype = &myArchetypeIndex[newType];
				numComponents++;
				columnIndex++;
			}

		}

		//Copying over component structure from old archetype to new archetype, no data is copied at this point.
		//Only copying meta data for component structure 
		newArchetype.ReserveComponentsSize(numComponents);
		newArchetype.SetMaxCount(2);
		newArchetype.ResizeComponents(numComponents);
		/*for (int i = 0; i < numComponents; i++)
		{
			newArchetype.components.emplace_back();
		}*/


		if (0 < aArchetypeSource.GetType().size())
		{
			for (int i = 0; i < aArchetypeSource.GetType().size(); ++i)
			{
				auto& archetypeMap = myComponentIndex.at(aArchetypeSource.GetType()[i]);

				int sourceColumnIndex = archetypeMap.at(aArchetypeSource.GetID()).columnIndex;
				int targetColumnIndex = archetypeMap.at(newArchetype.GetID()).columnIndex;
				if (sourceColumnIndex == -1 || targetColumnIndex == -1) continue; //Its a tag.

				newArchetype.GetColumn(targetColumnIndex)->AssignTypeInfo(aArchetypeSource.GetColumn(sourceColumnIndex)->GetTypeInfo());

				size_t elementSize = newArchetype.GetColumn(targetColumnIndex)->GetElementSize();
				size_t maxCount = newArchetype.GetMaxCount();
				newArchetype.GetColumn(targetColumnIndex)->Reset(new std::byte[elementSize * maxCount]);
				newArchetype.GetColumn(targetColumnIndex)->SetCapacity(elementSize * maxCount);
			}
		}


		//append new component to archetype
		if (!std::is_empty<T>())
		{
			ArchetypeMap& newArchetypeMap = myComponentIndex.at(componentID);
			int targetColumnIndex = newArchetypeMap.at(newArchetype.GetID()).columnIndex;

			Column* col = newArchetype.GetColumn(targetColumnIndex);
			col->AssignTypeInfo(RegisterComponent<T>());
			size_t maxCount = myArchetypeIndex[newType].GetMaxCount();
			size_t elementSize = col->GetElementSize();
			col->Reset(new std::byte[elementSize * maxCount]);
			col->SetCapacity(elementSize * maxCount);
		}

		ArchetypeEdge& edge = aArchetypeSource.AddEdge(componentID);
		edge.addArchetypes = &myArchetypeIndex[newType];

		ArchetypeEdge& newEdge = newArchetype.AddEdge(componentID);

		newEdge.removeArchetypes = &aArchetypeSource;
		newEdge.addArchetypes = nullptr;

		return myArchetypeIndex[newType];
	}

	template<typename T>
	ComponentID GetComponentID()
	{
		return typeid(T);
	}

}
