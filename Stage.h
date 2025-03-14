#pragma once
namespace ecs
{
	class Stage
	{
	public:
		void Create();
		void Merge();
		
	private:
		World* myWorld;
		std::unordered_map<ComponentID, ArchetypeMap> myComponentIndex; // Used to lookup components in archetypes
		std::unordered_map<Type, Archetype, TypeHash, TypeEqual> myArchetypeIndex; // Find an archetype by its list of component ids
		std::unordered_map<EntityID, Record> myEntityIndex;		// Find the archetype for an entity
	};

}