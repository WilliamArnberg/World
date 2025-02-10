#pragma once
#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>

#define ECS_ENTITY_NULL 0xDEADBABE

namespace ecs
{
	struct ArchetypeRecord;
	class Archetype;
	struct ArchetypeEdge;
	enum class ObserverType
	{
		OnSet,
		OnAdd,
		OnRemove
	};

	using ArchetypeID = uint64_t;
	using ComponentID = std::type_index;
	using EntityID = uint64_t;
	using Type = std::vector<ComponentID>; //This vector needs to be sorted 
	// Used to lookup components in archetypes
	using ArchetypeMap = std::unordered_map<ArchetypeID, ArchetypeRecord>;
	using ObserverList = std::vector<std::function<void()>>;
	using ObserverLists = std::unordered_map <ObserverType, ObserverList>;
	using ObserverRecord = std::unordered_map<EntityID, ObserverLists>;
	using ObserverMap = std::unordered_map<ComponentID, ObserverRecord>;
}