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
		Entity(EntityID aID, World* aWorld);
		/// <summary>
		/// Destroys the entity, removing it from the ECS and cleaning up its resources.
		/// </summary>
		/// <returns>
		/// Returns `true` if the entity was successfully destroyed, or `false` if there was an error or the entity could not be destroyed.
		/// </returns>
		bool Destroy();

		/// <summary>
		/// Checks if the entity is in a valid state or if it is null.
		/// </summary>
		/// <returns>
		/// Returns `true` if the entity is null or invalid, and `false` if the entity is valid.
		/// </returns>
		bool IsNull();

		/// <summary>
		/// Retrieves the ID of the entity.
		/// </summary>
		/// <returns>
		/// The entity ID.
		/// </returns>
		EntityID GetID() const;

		/// <summary>
		/// Checks if the entity has a component of the specified type.
		/// </summary>
		/// <typeparam name="T">The component type to check for.</typeparam>
		/// <returns>
		/// Returns `true` if the entity has the specified component, or `false` if it does not.
		/// </returns>
		template<typename T>
		bool HasComponent() const;

		/// <summary>
		/// Adds a component of the specified type to the entity.
		/// </summary>
		/// <typeparam name="T">The component type to add.</typeparam>
		/// <returns>
		/// A pointer to the added component <typeparamref name="T"/>.
		/// </returns>
		template<typename T>
		inline T* AddComponent();

		/// <summary>
		/// Removes the component of the specified type from the entity.
		/// </summary>
		template<typename T>
		inline void RemoveComponent();

		/// <summary>
		/// Retrieves a pointer to the component of the specified type attached to the entity.
		/// </summary>
		/// <typeparam name="T">The component type to retrieve.</typeparam>
		/// <returns>
		/// A pointer to the component of type <typeparamref name="T"/>, or `nullptr` if the component does not exist or is a tag.
		/// </returns>
		template<typename T>
		inline T* GetComponent();

		/// <summary>
		/// Marks an Entity to survive reloading a scene, this adds a component to the the entity aswell as setting all its parents to dont destroy.
		/// </summary>
		void SetDontDestroyOnLoad();

		/// <summary>
		/// Retrieves the entity's transform in world space, represented as a 4x4 matrix.
		/// </summary>
		/// <returns>
		/// A 4x4 matrix representing the entity's world transform.
		/// </returns>
		const JPH::Mat44 GetTransform();

		/// <summary>
		/// Retrieves the entity's transform in local space (relative to its parent), represented as a 4x4 matrix.
		/// </summary>
		/// <returns>
		/// A 4x4 matrix representing the entity's local transform.
		/// </returns>
		const JPH::Mat44 GetLocalTransform();

		/// <summary>
		/// Sets the entity's world transform using a 4x4 matrix.
		/// </summary>
		/// <param name="aTransform">The world transform to set, represented as a 4x4 matrix.</param>
		void SetWorldTransform(const JPH::Mat44& aTransform);

		/// <summary>
		/// Retrieves the entity's world position, represented as a vector in 3D space.
		/// </summary>
		/// <returns>
		/// A vector representing the entity's world position.
		/// </returns>
		JPH::Vec3 GetWorldPosition();

		/// <summary>
		/// Retrieves the entity's world rotation, represented as a quaternion.
		/// </summary>
		/// <returns>
		/// A quaternion representing the entity's world rotation.
		/// </returns>
		JPH::Quat GetWorldRotation();

		/// <summary>
		/// Retrieves the entity's world scale, represented as a vector in 3D space.
		/// </summary>
		/// <returns>
		/// A vector representing the entity's world scale.
		/// </returns>
		JPH::Vec3 GetWorldScale();

		friend std::ostream& operator<<(std::ostream& os, const Entity& aEntity);
	private:
		EntityID myID {0}; //
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
