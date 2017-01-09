#include <PhysXPlugin/PCH.h>
#include <PhysXPlugin/Components/PxDynamicActorComponent.h>
#include <PhysXPlugin/Components/PxCenterOfMassComponent.h>
#include <PhysXPlugin/WorldModule/PhysXWorldModule.h>
#include <PhysXPlugin/WorldModule/Implementation/PhysX.h>
#include <PhysXPlugin/Shapes/PxShapeComponent.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Core/WorldSerializer/WorldReader.h>


ezPxDynamicActorComponentManager::ezPxDynamicActorComponentManager(ezWorld* pWorld)
  : ezComponentManager<ezPxDynamicActorComponent, true>(pWorld)
{

}

ezPxDynamicActorComponentManager::~ezPxDynamicActorComponentManager()
{

}

void ezPxDynamicActorComponentManager::UpdateKinematicActors()
{
  EZ_PROFILE("KinematicActors");

  for (auto pKinematicActor : m_KinematicActors)
  {
    if (PxRigidDynamic* pActor = pKinematicActor->GetActor())
    {
      const auto pos = pKinematicActor->GetOwner()->GetGlobalPosition();
      const auto rot = pKinematicActor->GetOwner()->GetGlobalRotation();

      PxTransform t = PxTransform::createIdentity();
      t.p = PxVec3(pos.x, pos.y, pos.z);
      t.q = PxQuat(rot.v.x, rot.v.y, rot.v.z, rot.w);
      pActor->setKinematicTarget(t);
    }
  }
}

void ezPxDynamicActorComponentManager::UpdateDynamicActors(ezArrayPtr<const PxActiveTransform> activeTransforms)
{
  EZ_PROFILE("DynamicActors");

  for (auto& activeTransform : activeTransforms)
  {
    if (activeTransform.userData == nullptr)
      continue;

    PxTransform t = activeTransform.actor2World;

    ezVec3 pos(t.p.x, t.p.y, t.p.z);
    ezQuat rot(t.q.x, t.q.y, t.q.z, t.q.w);

    ezGameObject* pObject = static_cast<ezGameObject*>(activeTransform.userData);
    pObject->SetGlobalTransform(ezTransform(pos, rot));
  }
}

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_COMPONENT_TYPE(ezPxDynamicActorComponent, 1)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Kinematic", GetKinematic, SetKinematic),
    EZ_MEMBER_PROPERTY("Mass", m_fMass),
    EZ_MEMBER_PROPERTY("Density", m_fDensity)->AddAttributes(new ezDefaultValueAttribute(1.0f)),
    EZ_ACCESSOR_PROPERTY("DisableGravity", GetDisableGravity, SetDisableGravity),
    EZ_MEMBER_PROPERTY("LinearDamping", m_fLinearDamping)->AddAttributes(new ezDefaultValueAttribute(0.1f)),
    EZ_MEMBER_PROPERTY("AngularDamping", m_fAngularDamping)->AddAttributes(new ezDefaultValueAttribute(0.05f)),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

ezPxDynamicActorComponent::ezPxDynamicActorComponent()
{
  m_bKinematic = false;
  m_bDisableGravity = false;
  m_pActor = nullptr;

  m_fLinearDamping = 0.1f;
  m_fAngularDamping = 0.05f;
  m_fDensity = 1.0f;
  m_fMass = 0.0f;
}

void ezPxDynamicActorComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s << m_bKinematic;
  s << m_fDensity;
  s << m_fMass;
  s << m_bDisableGravity;
}

void ezPxDynamicActorComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());


  auto& s = stream.GetStream();

  s >> m_bKinematic;
  s >> m_fDensity;
  s >> m_fMass;
  s >> m_bDisableGravity;
}


void ezPxDynamicActorComponent::SetKinematic(bool b)
{
  if (m_bKinematic == b)
    return;

  m_bKinematic = b;

  if (m_bKinematic)
  {
    GetManager()->m_KinematicActors.PushBack(this);
  }
  else
  {
    GetManager()->m_KinematicActors.RemoveSwap(this);
  }

  if (m_pActor)
  {
    m_pActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_bKinematic);
  }
}

void ezPxDynamicActorComponent::SetDisableGravity(bool b)
{
  if (m_bDisableGravity == b)
    return;

  m_bDisableGravity = b;

  if (m_pActor)
  {
    m_pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, m_bDisableGravity);
  }
}

void ezPxDynamicActorComponent::OnSimulationStarted()
{
  ezPhysXWorldModule* pModule = GetWorld()->GetOrCreateModule<ezPhysXWorldModule>();

  const auto pos = GetOwner()->GetGlobalPosition();
  const auto rot = GetOwner()->GetGlobalRotation();

  PxTransform t = PxTransform::createIdentity();
  t.p = PxVec3(pos.x, pos.y, pos.z);
  t.q = PxQuat(rot.v.x, rot.v.y, rot.v.z, rot.w);
  m_pActor = ezPhysX::GetSingleton()->GetPhysXAPI()->createRigidDynamic(t);
  EZ_ASSERT_DEBUG(m_pActor != nullptr, "PhysX actor creation failed");

  m_pActor->userData = GetOwner();

  AddShapesFromObject(GetOwner(), m_pActor, GetOwner()->GetGlobalTransform());

  m_pActor->setLinearDamping(ezMath::Clamp(m_fLinearDamping, 0.0f, 1000.0f));
  m_pActor->setAngularDamping(ezMath::Clamp(m_fAngularDamping, 0.0f, 1000.0f));

  if (m_pActor->getNbShapes() == 0)
  {
    m_pActor->release();
    m_pActor = nullptr;

    ezLog::Error("Rigid Body '{0}' does not have any shape components. Actor will be removed.", GetOwner()->GetName());
    return;
  }

  ezVec3 vCoM(0.0f);
  if (FindCenterOfMass(GetOwner(), vCoM))
  {
    ezMat4 mTransform = GetOwner()->GetGlobalTransform().GetAsMat4();
    mTransform.Invert();

    vCoM = mTransform * vCoM;
  }

  if (m_fMass > 0.0f)
  {
    PxRigidBodyExt::setMassAndUpdateInertia(*m_pActor, m_fMass, (PxVec3*) &vCoM);
  }
  else if (m_fDensity > 0.0f)
  {
    PxRigidBodyExt::updateMassAndInertia(*m_pActor, m_fDensity, (PxVec3*)&vCoM);
  }
  else
  {
    ezLog::Warning("Rigid Body '{0}' neither has mass nor density set to valid values.", GetOwner()->GetName());
    PxRigidBodyExt::updateMassAndInertia(*m_pActor, 1.0f);
  }

  m_pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, m_bDisableGravity);
  m_pActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_bKinematic);

  if (m_bKinematic)
  {
    GetManager()->m_KinematicActors.PushBack(this);
  }

  {
    EZ_PX_WRITE_LOCK(*(pModule->GetPxScene()));
    pModule->GetPxScene()->addActor(*m_pActor);
  }
}

void ezPxDynamicActorComponent::Deinitialize()
{
  if (m_bKinematic)
  {
    GetManager()->m_KinematicActors.RemoveSwap(this);
  }

  if (m_pActor)
  {
    EZ_PX_WRITE_LOCK(*(m_pActor->getScene()));

    m_pActor->release();
    m_pActor = nullptr;
  }
}

bool ezPxDynamicActorComponent::FindCenterOfMass(ezGameObject* pRoot, ezVec3& out_CoM) const
{
  ezPxCenterOfMassComponent* pCOM;
  if (pRoot->TryGetComponentOfBaseType<ezPxCenterOfMassComponent>(pCOM))
  {
    out_CoM = pRoot->GetGlobalPosition();
    return true;
  }
  else
  {
    auto it = pRoot->GetChildren();

    while (it.IsValid())
    {
      if (FindCenterOfMass(it, out_CoM))
        return true;

      ++it;
    }
  }

  return false;
}
