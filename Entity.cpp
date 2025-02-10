#include "Entity.h"
#include "World.h"
namespace ecs
{
	Entity::Entity()
	{
		myID = 0xDEADBABE;
		myWorld = nullptr;
	}
	ecs::Entity::Entity(EntityID aID, World* aWorld)
	{
		myID = aID;
		myWorld = aWorld;
	}

	bool Entity::IsNull()
	{
		if(myID == 0xDEADBABE) return true;

		return false;
	}
	bool Entity::Destroy()
	{
		
		bool entityWasRemoved = myWorld->DestroyEntity(myID);
		myID = 0xDEADBABE;
		return entityWasRemoved;
	}
	EntityID Entity::GetID() const
	{
		return myID;
	}

	const JPH::Mat44 ecs::Entity::GetTransform()
	{
		auto parent = GetComponent<Parent>();
		if (!parent)
		{
			return GetLocalTransform();
		}
		else
		{
			return myWorld->GetEntity(parent->GetParent()).GetTransform() * GetLocalTransform();
		}
	}

	const JPH::Mat44 ecs::Entity::GetLocalTransform()
	{
		JPH::Mat44 localTransform = JPH::Mat44::sIdentity();
		localTransform.SetDiagonal3(JPH::Vec3(GetComponent<Scale>()->scale));
		localTransform = JPH::Mat44::sRotation(GetComponent<Rotation>()->rotation) * localTransform;
		localTransform.SetTranslation(JPH::Vec3(GetComponent<Position>()->position));
		return localTransform;
	}

	void Entity::SetWorldTransform(const JPH::Mat44& aTransform)
	{
		auto parent = GetComponent<Parent>();
		JPH::Mat44 localSpace = JPH::Mat44::sIdentity();
		if (parent)
		{
			localSpace = myWorld->GetEntity(parent->GetParent()).GetTransform().Inversed();
		}

		auto localTrans = localSpace * aTransform;

		GetComponent<Position>()->position = { localTrans.GetTranslation().GetX(), localTrans.GetTranslation().GetY(), localTrans.GetTranslation().GetZ() };

		localTrans.SetAxisX(localTrans.GetAxisX().Normalized());
		localTrans.SetAxisY(localTrans.GetAxisY().Normalized());
		localTrans.SetAxisZ(localTrans.GetAxisZ().Normalized());

		GetComponent<Rotation>()->rotation = localTrans.GetQuaternion();
	}

	JPH::Vec3 ecs::Entity::GetWorldPosition()
	{
		auto parent = GetComponent<Parent>();
		auto localPos = GetComponent<Position>();
		if (!parent)
		{
			return JPH::Vec3(localPos->position);
		}
		else
		{
			Entity myParent(parent->GetParent(), myWorld);
			return JPH::Vec3(localPos->position) + myParent.GetWorldPosition();
		}
	}

	JPH::Quat ecs::Entity::GetWorldRotation()
	{
		auto parent = GetComponent<Parent>();
		auto localRot = GetComponent<Rotation>();
		if (!parent)
		{
			return localRot->rotation;
		}
		Entity myParent(parent->GetParent(), myWorld);
		return localRot->rotation * myParent.GetWorldRotation();
	}

	JPH::Vec3 ecs::Entity::GetWorldScale()
	{
		auto parent = GetComponent<Parent>();
		auto scale = GetComponent<Scale>();
		if (!parent)
		{
			return JPH::Vec3(scale->scale);
		}
		Entity myParent(parent->GetParent(), myWorld);
		return JPH::Vec3(scale->scale) * myParent.GetWorldScale();
	}

	std::ostream& ecs::operator<<(std::ostream& os, const Entity& aEntity)
	{
		const Archetype* archetype = aEntity.myWorld->GetArchetype(aEntity.GetID());
		os << "\n" << "***********************************" << "\n";
		os << "Entity ID: " << aEntity.myID << "\n";
		os << "Archetype ID: " << archetype->GetID() << "\n";
		os << "Archetype Components: " << "\n";
		for (auto& type : archetype->GetType())
		{
			os << type.name() << "\n";
		}
		os << "***********************************" << "\n";

		return os;
	}

}