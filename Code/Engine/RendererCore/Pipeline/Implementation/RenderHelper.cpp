#include <RendererCore/PCH.h>
#include <RendererCore/RendererCore.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererFoundation/Context/Context.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Textures/TextureResource.h>
#include <RendererCore/ConstantBuffers/ConstantBufferResource.h>

ezMap<ezGALContext*, ezRendererCore::ContextState> ezRendererCore::s_ContextState;

// static
void ezRendererCore::SetMaterialState(ezGALContext* pContext, const ezMaterialResourceHandle& hMaterial)
{
  if (pContext == nullptr)
    pContext = ezGALDevice::GetDefaultDevice()->GetPrimaryContext();

  ezResourceLock<ezMaterialResource> pMaterial(hMaterial);

  ezRendererCore::SetActiveShader(pMaterial->GetShader(), pContext);
}

void ezRendererCore::OutputErrors(ezGALContext* pContext)
{
  if (pContext == nullptr)
    pContext = ezGALDevice::GetDefaultDevice()->GetPrimaryContext();

  ContextState& state = s_ContextState[pContext];

  if (state.m_uiFailedDrawcalls > 0)
  {
    ezLog::Error("%u drawcalls failed because of an invalid pipeline state", state.m_uiFailedDrawcalls);

    state.m_uiFailedDrawcalls = 0;
  }
}

// static 
void ezRendererCore::DrawMeshBuffer(ezGALContext* pContext, const ezMeshBufferResourceHandle& hMeshBuffer, ezUInt32 uiPrimitiveCount, ezUInt32 uiFirstPrimitive, ezUInt32 uiInstanceCount)
{
  if (pContext == nullptr)
    pContext = ezGALDevice::GetDefaultDevice()->GetPrimaryContext();

  if (ApplyContextStates(pContext).Failed())
  {
    ContextState& state = s_ContextState[pContext];
    state.m_uiFailedDrawcalls++;

    return;
  }

  EZ_ASSERT(uiFirstPrimitive < uiPrimitiveCount, "Invalid primitive range: first primitive (%d) can't be larger than number of primitives (%d)", uiFirstPrimitive, uiPrimitiveCount);

  ezResourceLock<ezMeshBufferResource> pMeshBuffer(hMeshBuffer);

  uiPrimitiveCount = ezMath::Min(uiPrimitiveCount, pMeshBuffer->GetPrimitiveCount() - uiFirstPrimitive);
  EZ_ASSERT(uiPrimitiveCount > 0, "Invalid primitive range: number of primitives can't be zero.");

  pContext->SetVertexBuffer(0, pMeshBuffer->GetVertexBuffer());

  ezGALBufferHandle hIndexBuffer = pMeshBuffer->GetIndexBuffer();
  if (!hIndexBuffer.IsInvalidated())
    pContext->SetIndexBuffer(hIndexBuffer);

  pContext->SetPrimitiveTopology(ezGALPrimitiveTopology::Triangles);
  uiPrimitiveCount *= 3;
  uiFirstPrimitive *= 3;

  {
    ContextState& state = s_ContextState[pContext];
    pContext->SetVertexDeclaration(GetVertexDeclaration(state.m_hActiveGALShader, pMeshBuffer->GetVertexDeclaration()));
  }

  if (uiInstanceCount > 1)
  {
    if (!hIndexBuffer.IsInvalidated())
    {
      pContext->DrawIndexedInstanced(uiPrimitiveCount, uiInstanceCount, uiFirstPrimitive);
    }
    else
    {
      pContext->DrawInstanced(uiPrimitiveCount, uiInstanceCount, uiFirstPrimitive);
    }
  }
  else
  {
    if (!hIndexBuffer.IsInvalidated())
    {
      pContext->DrawIndexed(uiPrimitiveCount, uiFirstPrimitive);
    }
    else
    {
      pContext->Draw(uiPrimitiveCount, uiFirstPrimitive);
    }
  }
}

void ezRendererCore::BindTexture(ezGALContext* pContext, const ezTempHashedString& sSlotName, const ezTextureResourceHandle& hTexture)
{
  if (pContext == nullptr)
    pContext = ezGALDevice::GetDefaultDevice()->GetPrimaryContext();

  ContextState& cs = s_ContextState[pContext];

  cs.m_BoundTextures[sSlotName.GetHash()] = hTexture;

  cs.m_bTextureBindingsChanged = true;
}

ezResult ezRendererCore::ApplyContextStates(ezGALContext* pContext, bool bForce)
{
  if (pContext == nullptr)
    pContext = ezGALDevice::GetDefaultDevice()->GetPrimaryContext();

  ContextState& state = s_ContextState[pContext];

  // make sure the internal state is up to date
  SetShaderContextState(pContext, state, bForce);

  if (!state.m_bShaderStateValid)
    return EZ_FAILURE;


  return EZ_SUCCESS;
}




EZ_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderHelper);

