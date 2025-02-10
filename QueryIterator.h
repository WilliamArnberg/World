#pragma once
#include "Entity.h"
#include <vector>
#include <unordered_map>
#include <cstdint>
namespace ecs
{
	class World;
	class Entity;

	class QueryIterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		QueryIterator() = default;
		QueryIterator(World* aWorld, std::vector<Archetype*> aArchetypeList);
		QueryIterator(std::vector<Archetype*> aArchetypeList, size_t aArchetypeIndex, size_t aEntityIndex, World* aWorld);
		QueryIterator(QueryIterator& aIterator);
		

		Entity operator*() const;
		Entity operator->(); 
		QueryIterator begin();
		QueryIterator end();

		// Prefix increment
		QueryIterator& operator++();
		// Postfix increment
		QueryIterator operator++(int);

		friend bool operator== (const QueryIterator& a, const QueryIterator& b);
		friend bool operator!= (const QueryIterator& a, const QueryIterator& b);
		size_t GetArchetypeIndex() const; 
		size_t GetEntityIndex() const; 

	private:
		std::vector<Archetype*> myArchetypes {};
		size_t myArchetypeIndex{};
		size_t myEntityIndex{};
		World* myWorld {nullptr};

	};


}
