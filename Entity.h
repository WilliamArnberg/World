#pragma once

#include <cstdint>
#include "Ecs_Aliases.h"
#include "World.h"
namespace ecs {
	class World;

	class Entity
	{
	public:
		Entity();
		Entity(entity aID, World* aWorld);
		bool Destroy();
		bool IsNull();
		entity GetID() const;
		
		template<typename T>
		bool HasComponent() const;

		//template<typename T, typename... args> //TODO: implement this but might need to have reflection system for it to function properly
		//T* Set(args&& arguments);

		template<typename T>
		inline T* AddComponent();
		template<typename T>
		inline void RemoveComponent();

		template<typename T>
		inline T* GetComponent();

		/*template<typename T>
		void EnableComponent(bool aEnabled);

		template<typename T>
		bool ComponentIsDisabled();*/

		//Returns the transform in worldposition
		const JPH::Mat44 GetTransform();
		const JPH::Mat44 GetLocalTransform();

		void SetWorldTransform(const JPH::Mat44& aTransform);

		JPH::Vec3 GetWorldPosition();
		JPH::Quat  GetWorldRotation();
		JPH::Vec3 GetWorldScale();

		friend std::ostream& operator<<(std::ostream& os, const Entity& aEntity);
	private:
		entity myID; //
		World* myWorld;
	};
	template <typename T>
	bool Entity::HasComponent() const
	{
		return myWorld->HasComponent<T>(myID);
	}
	template <typename T>
	T* Entity::AddComponent() {
		return myWorld->AddComponent<T>(myID);
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		myWorld->RemoveComponent<T>(myID);
	}

	template <typename T>
	T* Entity::GetComponent() {
		return myWorld->GetComponent<T>(myID);
	}
}
