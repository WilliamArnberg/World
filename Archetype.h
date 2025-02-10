#pragma once
#include <cassert>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <ostream>
#include <vector>

#include "Ecs_Aliases.h"
namespace ecs
{

	struct TypeHash
	{
		size_t operator()(const std::vector<std::type_index>& types) const
		{
			size_t seed = types.size();
			for (const auto& type : types)
			{
				seed ^= type.hash_code() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
	struct TypeEqual
	{
		bool operator()(const std::vector<std::type_index>& lhs, const std::vector<std::type_index>& rhs) const
		{
			return lhs == rhs;
		}
	};
	struct ArchetypeRecord
	{
		Archetype* archetype;
		int columnIndex;
	};

	struct ComponentTypeInfo
	{
		ComponentID typeID;
		size_t size = 0;											// Size of the component type in bytes
		size_t alignment = 0;										// Alignment requirement of the type
		void (*construct)(void* dest) = nullptr;					// Function pointer for default construction
		void (*copy)(void* dest, const void* src) = nullptr;		// copy constructor
		void (*move)(void* dest, void* src) = nullptr;				// Move constructor
		void (*destruct)(void* obj) = nullptr;						// Destructor
		bool isTrivial = false;

    ComponentTypeInfo()
        : typeID(typeid(nullptr)), size(0), alignment(0), construct(nullptr), copy(nullptr), 
          move(nullptr), destruct(nullptr), isTrivial(false) {}

    ComponentTypeInfo(const ComponentTypeInfo& aOther)
        : typeID(aOther.typeID), size(aOther.size), alignment(aOther.alignment),
          construct(aOther.construct), copy(aOther.copy), move(aOther.move),
          destruct(aOther.destruct), isTrivial(aOther.isTrivial) {}

    ComponentTypeInfo(ComponentTypeInfo&& aOther) noexcept
        : typeID(aOther.typeID), size(aOther.size), alignment(aOther.alignment),
          construct(aOther.construct), copy(aOther.copy), move(aOther.move),
          destruct(aOther.destruct), isTrivial(aOther.isTrivial)
    {
        aOther.construct = nullptr;
        aOther.copy = nullptr;
        aOther.move = nullptr;
        aOther.destruct = nullptr;
        aOther.isTrivial = false;
    }

    ComponentTypeInfo& operator=(const ComponentTypeInfo& aOther)
    {
        if (this != &aOther)  
        {
            
            typeID = aOther.typeID;
            size = aOther.size;
            alignment = aOther.alignment;
            construct = aOther.construct;
            copy = aOther.copy;
            move = aOther.move;
            destruct = aOther.destruct;
            isTrivial = aOther.isTrivial;
        }
        return *this;
    }


    ComponentTypeInfo& operator=(ComponentTypeInfo&& aOther) noexcept
    {
        if (this != &aOther)  
        {
            typeID = aOther.typeID;
            size = aOther.size;
            alignment = aOther.alignment;
            construct = aOther.construct;
            copy = aOther.copy;
            move = aOther.move;
            destruct = aOther.destruct;
            isTrivial = aOther.isTrivial;

            aOther.construct = nullptr;
            aOther.copy = nullptr;
            aOther.move = nullptr;
            aOther.destruct = nullptr;
            aOther.isTrivial = false;
        }
        return *this;
    }

	};

	class Column
	{
	public:
		Column() = default;
		~Column()
		{
			myBuffer.reset();
		};
		Column(Column&& other) noexcept
		{
			myBuffer = std::move(other.myBuffer);
			myCapacity = other.myCapacity;
			myCurrentMemoryUsed = other.myCurrentMemoryUsed;
			myTypeInfo = other.myTypeInfo;
		}
		Column& operator=(Column&& other) noexcept
		{
			if (this != &other)
			{
				myBuffer = std::move(other.myBuffer);
				myCapacity = other.myCapacity;
				myCurrentMemoryUsed = other.myCurrentMemoryUsed;
				myTypeInfo = other.myTypeInfo;
			}
			return *this;
		}
		Column(const Column& aOther)
		{
			myBuffer.reset(aOther.myBuffer.get());
			myCapacity = aOther.myCapacity;
			myCurrentMemoryUsed = aOther.myCurrentMemoryUsed;
			myTypeInfo = aOther.myTypeInfo;
		};
		Column& operator=(const Column&)
		{
			return *this;
		};

		size_t GetElementSize() const;
		size_t GetCurrentMemoryUsed() const;
		size_t GetCapacity() const;
		void SetCapacity(size_t aCapacity);
		void Reset(std::byte* aBuffer);
		void Resize(size_t aMul);
		void AssignTypeInfo(const ComponentTypeInfo& aTypeInfo);
		void ChangeMemoryUsed(int aNumElements);
		const ComponentTypeInfo& GetTypeInfo() const;
		std::byte* Release();
		void* GetComponent(size_t aIndex) const;
		void* operator[](const size_t aIndex) const
		{
			assert(aIndex <= (myCapacity), "Trying to access element outside of buffer");
			return myBuffer.get() + (aIndex * GetElementSize());
		}

	private:
		std::unique_ptr<std::byte[]> myBuffer; //Component storage
		ComponentTypeInfo myTypeInfo;
		size_t myCapacity = 0;
		size_t myCurrentMemoryUsed = 0;

	};

	class Archetype
	{
	public:
		Archetype() = default;
		~Archetype() = default;
		ArchetypeID		GetID() const;
		void			SetID(ArchetypeID aID);
		const Type&		GetType() const;
		ComponentID		GetComponentIDFromTypeList(size_t aIndex) const;
		size_t			GetNumTypes() const;
		size_t			GetMaxCount() const;
		void			SetType(const Type& aType);
		void			SetMaxCount(size_t aMaxCount);
		Column*			GetColumn(size_t aColumnIndex);
		size_t			GetNumComponents() const;
		size_t			GetComponentCapacity() const;
		ecs::EntityID		GetEntity(size_t aRow) const;
		void			AddComponentIDToTypeSet(ComponentID aComponentID);
		bool			HasComponent(ComponentID aComponentID) const;
		size_t			GetNumEntities() const;
		ArchetypeEdge&	GetEdge(ComponentID aID);
		ArchetypeEdge&  GetOrAddEdge(ComponentID aID);
		void			ReserveComponentsSize(size_t aSize);
		void			ResizeComponents(size_t aSize);
		bool			IsEmpty() const;
		void			Reset();
		void			AddEmptyComp();
		std::vector<EntityID>& GetEntityList();
		
		void			AddEntity(ecs::EntityID aEntity);
		bool			Contains(const Type& type) const;

		template <typename... Filter>
		bool			Contains(std::tuple<Filter...> filters) const;
		ArchetypeEdge& AddEdge(ComponentID aComponentID);
		int			FindColumnIndex(ComponentID aComponentID) const;
	private:

		ArchetypeID myID{ 0 };
		Type myType{};						//The order of components in the component list
		std::unordered_set<ComponentID> typeSet{};
		std::vector<Column> components{}; //Columns holding the data, use the entity row to access the specific component
		std::vector<EntityID> entities{}; //serves as our entity list but the order of entities are also the rows in the component columns
		std::unordered_map<ComponentID, ArchetypeEdge> edges{};
		size_t myMaxCount = size_t(0);

		friend std::ostream& operator<<(std::ostream& os, const Archetype& aArchetype);

	};


	struct ArchetypeEdge
	{
		ArchetypeEdge() = default;

		Archetype* addArchetypes = nullptr;
		Archetype* removeArchetypes = nullptr;

	};
	struct Record
	{
		Archetype* archetype;
		size_t row = 0;
	};
	template<typename ...Filter>
	inline bool Archetype::Contains(std::tuple<Filter...> filters) const
	{
		return std::apply(
			[this](auto&&... args)
			{
			// Return false immediately if any of the components is not present
			return (HasComponent(typeid(args)) && ...);

			}, filters);
	}
}
