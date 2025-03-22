# Entity-Component-System
 Entity-Component-System for C++20.
 The project was built during my second year at The Game Assembly in Stockholm.


## Features

✔️ Cache-Friendly archetype and SoA (Struct of Arrays) storage.  <br />
✔️ Handles POD & non POD datatypes, either by letting the compiler auto generate constructors for you or write your own. <br />
✔️ Write free floating queries or add functions to systems that automate and structure the pipelining. <br />
✔️ Easy to type Queries that return an range-for iterator returning a view class to each entity in that query spanning across multiple archetypes. <br />
✔️ Filtered Queries for when you need all entities containing N types as long as they don't contain M types.  
Returns an range-for iterator returning a view class to each entity in that query spanning across multiple archetypes. <br />
✔️ Cached Queries, that only gets reset if the underlying memory of the archetype changes. <br />
🔜: (WIP) Staging and merging to allow multi-threaded loading of assets into the Entity-Component-System <br />
## Core Concepts
### Entity
```cpp
    using entity = uint64_t;
```
An entity is a unique identifier that represents a game object. It does not contain any data or logic itself but serves as a reference for components.

### Components
Components are user created [POD](https://learn.microsoft.com/en-us/cpp/cpp/trivial-standard-layout-and-pod-types?view=msvc-170#pod-types) or Non-POD data structures that store information about an entity. 

### Component Storage

Each Component type is stored in a column which consists of a contiguous data buffer, and type-erasure information.

```cpp
class Column 
{
    std::unique_ptr<std::byte[]> myBuffer; //Component storage
	ComponentTypeInfo myTypeInfo; //Type Info
	size_t myCapacity{0}; //Amount of bytes this column can hold before needing to grow
	size_t myCurrentMemoryUsed{0}; 
}
```
The Type erasure data each column hold is filled up automatically when the user calls `AddComponent<T>();` 
Storing type-erased constructors enables the usage of modern C++ functionalities.
```cpp
struct ComponentTypeInfo
{
    ComponentID typeID;
	size_t size{0};											//Size of the component type in bytes
	size_t alignment {0};										//Alignment requirement of the type
	void (*construct)(void* aDest) = nullptr; //Function pointer for default construction
	void (*copy)(void* aDest, const void* aSrc) = nullptr;//copy constructor
	void (*move)(void* aDest, void* aSrc) = nullptr;// Move constructor
	void (*destruct)(void* aObj) = nullptr;// Destructor
	bool isTrivial = false;
}
```

The columns are stored in an Archetype.

```cpp
struct Archetype 
{
    ArchetypeID myID{ 0 };
    Type myType{};	//The order of components in the component list
    std::unordered_set<ComponentID> myTypeSet{}; //Used for fast lookup into the archetype if it contains a specific type
    std::vector<Column> myComponents{}; //Columns holding the data, use the entity row to access the specific component
    std::vector<entity> myEntities{}; //serves as our entity list but the order of entities are also the rows in the component columns
    std::unordered_map<ComponentID, ArchetypeEdge> myEdges{}; //Add and remove Edges.
    size_t myMaxCount{0};
}
```

### Systems
Systems are functions with queries.

```cpp
//Systems can be added

World.system("Move Entity",[]()
{
    for(Entity entity : World.Query<Position,Rotation,Scale,Velocity>())
    {

        Position* position = entity.GetComponent<Position>();   
        Velocity* velocity = entity.GetComponent<Velocity>();
        position += velocity;   

    }   
},ecs::Pipeline::OnUpdate);

//Systems can be removed
World.RemoveSystem("Move Entity",ecs::Pipeline::OnUpdate);
```
Removing a system puts it into a queue where it will get removed from the SystemManager at the end of the frame.

Pipelining the systems empowers every decision about the data we are operating on as we can for a fact know in what state each data is at every point of execution in our codebase.


### Staging
Staging allows asynchronous management and execution of code. 
A stage is a new world, however this world may at any point be merged with another to resume execution in the original.
This allows for example level streaming where levels and/or parts of the current level may be loaded asynchronously, code and anything else may be executed on the stage as it is essentially just copy of the World.
````cpp

SceneManager sceneManager; /example code
World.CreateStage("SomeStage"); 

sceneManager.loadScene();
Stage* stage = World.GetStage("SomeStage");
stage->Merge();

````





### Example Usage
```cpp
#include "World.h"
struct Position {
  float x, y, z;
};

struct Rotation {
  Quaternion quat;
};

struct Scale {
  float x, y, z;
};

int main(int argc, char *argv[])
{
  using namespace ecs;
  World world;

  auto entity = world.Create();
  entity.AddComponent<Position>();
  entity.AddComponent<Rotation>();
  entity.AddComponent<Scale>();
  
  world.system("Example",[&world]()
  {
     for(auto e : world.query<Position,Rotation,Scale>())
     {
         auto pos = e.GetComponent<Position>();
         auto rot = e.GetComponent<Rotation>();
         auto scale = e.GetComponent<Scale>();
         //Do something.
  
     }
  },ecs::Pipeline::OnUpdate);
  
  while(world.progress()){}; 

}
```
