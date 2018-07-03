#include <PCH.h>

#include <Core/World/World.h>
#include <Foundation/Profiling/Profiling.h>
#include <ParticlePlugin/Effect/ParticleEffectInstance.h>
#include <ParticlePlugin/Type/Mesh/ParticleTypeMesh.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Textures/Texture2DResource.h>

// clang-format off
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleTypeMeshFactory, 1, ezRTTIDefaultAllocator<ezParticleTypeMeshFactory>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Mesh", m_sMesh)->AddAttributes(new ezAssetBrowserAttribute("Mesh")),
    EZ_MEMBER_PROPERTY("TintColorParam", m_sTintColorParameter),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezParticleTypeMesh, 1, ezRTTIDefaultAllocator<ezParticleTypeMesh>)
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

const ezRTTI* ezParticleTypeMeshFactory::GetTypeType() const
{
  return ezGetStaticRTTI<ezParticleTypeMesh>();
}


void ezParticleTypeMeshFactory::CopyTypeProperties(ezParticleType* pObject) const
{
  ezParticleTypeMesh* pType = static_cast<ezParticleTypeMesh*>(pObject);

  pType->m_hMesh.Invalidate();
  pType->m_sTintColorParameter = ezTempHashedString(m_sTintColorParameter.GetData());

  if (!m_sMesh.IsEmpty())
    pType->m_hMesh = ezResourceManager::LoadResource<ezMeshResource>(m_sMesh);
}

enum class TypeMeshVersion
{
  Version_0 = 0,
  Version_1,

  // insert new version numbers above
  Version_Count,
  Version_Current = Version_Count - 1
};

void ezParticleTypeMeshFactory::Save(ezStreamWriter& stream) const
{
  const ezUInt8 uiVersion = (int)TypeMeshVersion::Version_Current;
  stream << uiVersion;

  stream << m_sMesh;
  stream << m_sTintColorParameter;
}

void ezParticleTypeMeshFactory::Load(ezStreamReader& stream)
{
  ezUInt8 uiVersion = 0;
  stream >> uiVersion;

  EZ_ASSERT_DEV(uiVersion <= (int)TypeMeshVersion::Version_Current, "Invalid version {0}", uiVersion);

  stream >> m_sMesh;
  stream >> m_sTintColorParameter;
}

ezParticleTypeMesh::ezParticleTypeMesh() = default;
ezParticleTypeMesh::~ezParticleTypeMesh() = default;

void ezParticleTypeMesh::CreateRequiredStreams()
{
  CreateStream("Position", ezProcessingStream::DataType::Float4, &m_pStreamPosition, false);
  CreateStream("Size", ezProcessingStream::DataType::Float, &m_pStreamSize, false);
  CreateStream("Color", ezProcessingStream::DataType::Float4, &m_pStreamColor, false);
  CreateStream("RotationSpeed", ezProcessingStream::DataType::Float, &m_pStreamRotationSpeed, false);
  CreateStream("Axis", ezProcessingStream::DataType::Float3, &m_pStreamAxis, true);
}

void ezParticleTypeMesh::InitializeElements(ezUInt64 uiStartIndex, ezUInt64 uiNumElements)
{
  ezVec3* pAxis = m_pStreamAxis->GetWritableData<ezVec3>();
  ezRandom& rng = GetRNG();

  //if (m_RotationAxis == ezSpriteAxis::Random)
  {
    for (ezUInt32 i = 0; i < uiNumElements; ++i)
    {
      const ezUInt64 uiElementIdx = uiStartIndex + i;

      pAxis[uiElementIdx] = ezVec3::CreateRandomDirection(rng);
    }
  }
}

void ezParticleTypeMesh::ExtractTypeRenderData(const ezView& view, ezExtractedRenderData& extractedRenderData, const ezTransform& instanceTransform, ezUInt64 uiExtractedFrame) const
{
  EZ_PROFILE("PFX: Mesh");

  if (!m_hMesh.IsValid())
    return;

  const ezUInt32 numParticles = (ezUInt32)GetOwnerSystem()->GetNumActiveParticles();

  if (numParticles == 0)
    return;

  const ezTime tCur = GetOwnerSystem()->GetWorld()->GetClock().GetAccumulatedTime();

  // don't copy the data multiple times in the same frame, if the effect is instanced
  //if (m_uiLastExtractedFrame != uiExtractedFrame)
  {
    const ezColor tintColor = GetOwnerEffect()->GetColorParameter(m_sTintColorParameter, ezColor::White);

    m_uiLastExtractedFrame = uiExtractedFrame;

    const ezVec4* pPosition = m_pStreamPosition->GetData<ezVec4>();
    const float* pSize = m_pStreamSize->GetData<float>();
    const ezColor* pColor = m_pStreamColor->GetData<ezColor>();
    const float* pRotationSpeed = m_pStreamRotationSpeed->GetData<float>();
    const ezVec3* pAxis = m_pStreamAxis->GetData<ezVec3>();

    {
      ezResourceLock<ezMeshResource> pMesh(m_hMesh);

      ezBoundingBoxSphere bounds = pMesh->GetBounds();
      const ezMeshResourceDescriptor::SubMesh& subMesh = pMesh->GetSubMeshes()[0];
      const ezUInt32 uiMeshIDHash = m_hMesh.GetResourceIDHash();
      ezMaterialResourceHandle hMaterial = pMesh->GetMaterials()[0];
      const ezUInt32 uiMaterialIDHash = hMaterial.IsValid() ? hMaterial.GetResourceIDHash() : 0;

      // Generate batch id from mesh, material and part index.
      ezUInt32 data[] = {uiMeshIDHash, uiMaterialIDHash, 0, 0};
      ezUInt32 uiBatchId = ezHashing::xxHash32(data, sizeof(data));
      ezUInt32 uiFlipWinding = 0;

      for (ezUInt32 p = 0; p < numParticles; ++p)
      {
        const ezUInt32 idx = p;

        ezTransform trans;
        trans.m_qRotation.SetFromAxisAndAngle(pAxis[p], ezAngle::Radian((float)(tCur.GetSeconds() * pRotationSpeed[idx])));
        trans.m_vPosition = pPosition[idx].GetAsVec3();
        trans.m_vScale.Set(pSize[idx]);

        ezMeshRenderData* pRenderData = ezCreateRenderDataForThisFrame<ezMeshRenderData>(nullptr, uiBatchId);
        {
          pRenderData->m_GlobalTransform = trans;
          pRenderData->m_GlobalBounds = bounds;
          pRenderData->m_hMesh = m_hMesh;
          pRenderData->m_hMaterial = hMaterial;
          pRenderData->m_Color = pColor[idx] * tintColor;

          pRenderData->m_uiPartIndex = 0;
          pRenderData->m_uiFlipWinding = uiFlipWinding;
          pRenderData->m_uiUniformScale = 1;

          pRenderData->m_uiUniqueID = 0xFFFFFFFF;
        }

        // Sort by material and then by mesh
        ezUInt32 uiSortingKey = (uiMaterialIDHash << 16) | (uiMeshIDHash & 0xFFFE) | uiFlipWinding;
        extractedRenderData.AddRenderData(pRenderData, ezDefaultRenderDataCategories::LitOpaque, uiSortingKey);
      }
    }
  }
}

