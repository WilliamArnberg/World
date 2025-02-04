# Entity-Component-System
 Entity-Component-System for C++20.

**Features**

✔️ Cache-Friendly archetype and SoA (Struct of Arrays) storage.  <br />
✔️ Handles POD & non POD datatypes, either by letting the compiler auto generate constructors for you or write your own. <br />
✔️ Write free floating queries or add functions to systems that automate and structure the pipelining. <br />


```cpp
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
