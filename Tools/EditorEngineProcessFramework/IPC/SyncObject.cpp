﻿#include <PCH.h>
#include <EditorEngineProcessFramework/IPC/SyncObject.h>
#include <ToolsFoundation/Document/Document.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <EditorEngineProcessFramework/EngineProcess/EngineProcessDocumentContext.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezEditorEngineSyncObject, 1, ezRTTINoAllocator)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("SyncGuid", m_SyncObjectGuid),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

ezEditorEngineSyncObject::ezEditorEngineSyncObject()
{
  m_SyncObjectGuid.CreateNewUuid();
  m_bModified = true;
}

ezEditorEngineSyncObject::~ezEditorEngineSyncObject()
{
  if (m_OnDestruction.IsValid())
  {
    m_OnDestruction(this);
  }
}

void ezEditorEngineSyncObject::Configure(ezUuid ownerGuid, ezDelegate<void(ezEditorEngineSyncObject*)> onDestruction)
{
  m_OwnerGuid = ownerGuid;
  m_OnDestruction = onDestruction;
}

ezUuid ezEditorEngineSyncObject::GetDocumentGuid() const
{
  return m_OwnerGuid;
}
