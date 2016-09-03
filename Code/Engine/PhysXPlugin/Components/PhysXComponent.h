#pragma once

#include <PhysXPlugin/Basics.h>
#include <Core/World/World.h>
#include <Core/World/Component.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>

#include <PxPhysicsAPI.h>
using namespace physx;

/// \brief Base class for all PhysX components, such that they all have a common ancestor
class EZ_PHYSXPLUGIN_DLL ezPhysXComponent : public ezComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezPhysXComponent, ezComponent);
  virtual void ezPhysXComponentIsAbstract() = 0; // abstract classes are not shown in the UI, since this class has no other abstract functions so far, this is a dummy

public:
  ezPhysXComponent() {}

  virtual void SerializeComponent(ezWorldWriter& stream) const override {}
  virtual void DeserializeComponent(ezWorldReader& stream) override {}

  // ************************************* PROPERTIES ***********************************


protected:
  //ezBitflags<ezTransformComponentFlags> m_Flags;

  // ************************************* FUNCTIONS *****************************

public:


};
