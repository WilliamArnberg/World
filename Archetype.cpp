#include "Archetype.h"
namespace ecs
{

	ecs::ArchetypeID ecs::Archetype::GetID() const
	{
		return myID;
	}

	void Archetype::SetID(ArchetypeID aID)
	{
		myID = aID;
	}

	const Type& Archetype::GetType() const
	{
		return myType;
	}

	ComponentID Archetype::GetComponentIDFromTypeList(size_t aIndex) const
	{
		return myType.at(aIndex);
	}

	size_t Archetype::GetNumTypes() const
	{
		return myType.size();
	}

	size_t Archetype::GetMaxCount() const
	{
		return myMaxCount;
	}

	void Archetype::SetType(const Type& aType)
	{
		myType = aType;
	}

	void Archetype::SetMaxCount(size_t aMaxCount)
	{
		myMaxCount = aMaxCount;
	}

	Column* Archetype::GetColumn(size_t aColumnIndex)
	{
		return &components[aColumnIndex];
	}

	size_t Archetype::GetComponentCapacity() const
	{
		return components.capacity();
	}

	ecs::entity Archetype::GetEntity(size_t aRow) const
	{
		return entities.at(aRow);
	}

	void Archetype::AddComponentIDToTypeSet(ComponentID aComponentID)
	{
		typeSet.insert(aComponentID);
	}

	bool Archetype::HasComponent(ComponentID aComponentID) const
	{
		return typeSet.count(aComponentID);
	}

	size_t Archetype::GetNumEntities() const
	{
		return entities.size();
	}

	void Archetype::ReserveComponentsSize(size_t aSize)
	{
		components.reserve(aSize);
	}

	void Archetype::ResizeComponents(size_t aSize)
	{
		components.resize(aSize);
	}

	bool Archetype::IsEmpty() const
	{
		return entities.empty();
	}

	void Archetype::Reset()
	{
		int myPreviousCount = (int)entities.size();
		entities.clear();
		myMaxCount = 2;
		for (auto& comp : components)
		{
			comp.SetCapacity(myMaxCount * comp.GetElementSize());
			comp.Reset(nullptr);
			comp.Reset(new std::byte[comp.GetElementSize() * comp.GetCapacity()]);

			comp.ChangeMemoryUsed(-myPreviousCount);
		}
	}

	void Archetype::AddEmptyComp()
	{
		components.emplace_back();
	}



	std::vector<entity>& Archetype::GetEntityList()
	{
		return entities;
	}

	void Archetype::AddEntity(ecs::entity aEntity)
	{
		entities.emplace_back(aEntity);
	}



	

	bool Archetype::Contains(const Type& type) const
	{
		for (auto& t : type)
		{
			if (HasComponent(t) == false) return false;
		}
		return true;
	}

	ArchetypeEdge& Archetype::AddEdge(ComponentID aComponentID)
	{
		return edges[aComponentID];
	}

	int Archetype::FindColumnIndex(ComponentID aComponentID) const
	{
		for (int i = 0; i < myType.size(); i++)
		{
			if (aComponentID != myType[i]) continue;

			return i;
		}

		return -1;
	}

	ArchetypeEdge& Archetype::GetEdge(ComponentID aID)
	{
		return edges.at(aID);
	}

	ArchetypeEdge& Archetype::GetOrAddEdge(ComponentID aID)
	{
		return edges[aID];
	}

	size_t Archetype::GetNumComponents() const
	{
		return components.size();
	}



	size_t ecs::Column::GetElementSize() const
	{
		return  myTypeInfo.size;
	}

	size_t ecs::Column::GetCurrentMemoryUsed() const
	{
		return myCurrentMemoryUsed;
	}

	size_t ecs::Column::GetCapacity() const
	{
		return myCapacity;
	}

	void ecs::Column::SetCapacity(size_t aCapacity)
	{
		myCapacity = aCapacity;
	}

	void ecs::Column::Reset(std::byte* aBuffer)
	{
		myBuffer.reset(aBuffer);
	}

	void ecs::Column::Resize(size_t aMul)
	{
		size_t newSize = GetCapacity() * aMul;
		std::unique_ptr<std::byte[]> newData(new std::byte[newSize]);

		std::memcpy(newData.get(), myBuffer.get(), GetCapacity());
		SetCapacity(newSize);
		myBuffer = std::move(newData);
	}

	void ecs::Column::AssignTypeInfo(const ComponentTypeInfo& aTypeInfo)
	{
		myTypeInfo = aTypeInfo;
	}

	//Increment or decrement the amount of memory used.
	void ecs::Column::ChangeMemoryUsed(int aNumElements)
	{
		myCurrentMemoryUsed += aNumElements * myTypeInfo.size;
	}

	const ecs::ComponentTypeInfo& ecs::Column::GetTypeInfo() const
	{
		return myTypeInfo;
	}

	std::byte* ecs::Column::Release()
	{
		return myBuffer.release();
	}

	void* Column::GetComponent(size_t aIndex) const
	{
		assert(aIndex <= (GetCapacity()), "Trying to access element outside of buffer");
		return myBuffer.get() + (aIndex * GetElementSize());;
	}

}

