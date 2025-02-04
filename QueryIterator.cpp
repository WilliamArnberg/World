#include "QueryIterator.h"
#include "World/World.h"
namespace ecs {
	bool operator==(const QueryIterator& a, const QueryIterator& b)
	{
		return a.GetArchetypeIndex() == b.GetArchetypeIndex() && a.GetEntityIndex() == b.GetEntityIndex();

	}

	bool operator!=(const QueryIterator& a, const QueryIterator& b)
	{
		return !(a == b);
	}

	Entity QueryIterator::operator*() const
	{
		return { 
			myArchetypes.at(myArchetypeIndex)->GetEntityList().at(myEntityIndex), myWorld 
		};
	}

	ecs::QueryIterator::QueryIterator(World* aWorld, std::vector<Archetype*> aArchetypeList)
		: myArchetypes(std::move(aArchetypeList)), myArchetypeIndex(0), myEntityIndex(0), myWorld(aWorld), mySize(0)
	{
		for (const auto& a : myArchetypes)
		{
			mySize += a->GetNumEntities();
		}
	}

	ecs::QueryIterator::QueryIterator(std::vector<Archetype*> aArchetypeList, size_t aArchetypeIndex, size_t aEntityIndex, World* aWorld)
		: myArchetypes(std::move(aArchetypeList)), myArchetypeIndex(aArchetypeIndex), myEntityIndex(aEntityIndex), myWorld(aWorld), mySize(0)
	{
		for (const auto& a : myArchetypes)
		{
			mySize += a->GetNumEntities();
		}
	}

	QueryIterator::QueryIterator(QueryIterator& aIterator) 
		: myArchetypeIndex(aIterator.myArchetypeIndex), myEntityIndex(aIterator.myEntityIndex), myWorld(aIterator.myWorld), mySize(0)
	{
			
		{
			for (const auto& a : myArchetypes)
			{
				mySize += a->GetNumEntities();
			}
		}
	}

	ecs::QueryIterator ecs::QueryIterator::operator++(int)
	{
		myEntityIndex++;
		if (myEntityIndex >= myArchetypes.at(myArchetypeIndex)->GetNumEntities())
		{
			myArchetypeIndex++;
			myEntityIndex = 0;
		}

		return *this;
	}

	size_t QueryIterator::GetArchetypeIndex() const
	{
		return myArchetypeIndex; 
	}

	size_t QueryIterator::GetEntityIndex() const
	{
		return myEntityIndex;
	}

	size_t QueryIterator::GetElementSize() const
	{
		return mySize;
	}

	ecs::QueryIterator& ecs::QueryIterator::operator++()
	{
		++myEntityIndex;
		if (myEntityIndex >= myArchetypes.at(myArchetypeIndex)->GetNumEntities())
		{
			myArchetypeIndex++;
			myEntityIndex = 0;
		}
		return *this;
	}
	
	Entity QueryIterator::operator->()
	{
		return { myArchetypes.at(myArchetypeIndex)->GetEntityList().at(myEntityIndex), myWorld }; 
	}

	ecs::QueryIterator ecs::QueryIterator::begin()
	{
		return { myArchetypes,0,0,myWorld };
	}

	ecs::QueryIterator ecs::QueryIterator::end()
	{
		if (myArchetypes.size() < 1)
		{
			return { myArchetypes,0,0,myWorld };
		}

		return { myArchetypes,myArchetypes.size(),0,myWorld };

	}

}
