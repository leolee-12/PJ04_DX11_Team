#include "LensFlare.h"
#include "GameContent_const.h"
#include "RectEmitterCommon.h"
#include "MeshEmitterCommon.h"

CLensFlare::CLensFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
    , m_bUseScreenAxis{ true }
    , m_fAxisSourceZ{ 40.f }
    , m_fAxisOppositeZ{ 110.f }
    , m_fAxisExtent{ 2.f }
    , m_fViewDepthScale{ 1.f }
    , m_fScreenCullMargin{ 1.25f }
{
}

CLensFlare::CLensFlare(const CLensFlare& Prototype)
    : CEffect_Container(Prototype)
    , m_bUseScreenAxis{ Prototype.m_bUseScreenAxis }
    , m_fAxisSourceZ{ Prototype.m_fAxisSourceZ }
    , m_fAxisOppositeZ{ Prototype.m_fAxisOppositeZ }
    , m_fAxisExtent{ Prototype.m_fAxisExtent }
    , m_fViewDepthScale{ Prototype.m_fViewDepthScale }
    , m_fScreenCullMargin{ Prototype.m_fScreenCullMargin }
{
}

HRESULT CLensFlare::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLensFlare::Initialize(void* pArg)
{
    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CLensFlare::On_Deserialized()
{
    __super::On_Deserialized();
    Cache_LensElements();
}

void CLensFlare::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CLensFlare::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CLensFlare::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    Compute_CombinedWorldMatrix();

    if (m_bLensElementCacheReady == false)
        Cache_LensElements();

    if (m_bLensElementCacheReady == false)
        return;

    if (Update_LensFlarePlacement() == false)
        return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CLensFlare::Render()
{
    return __super::Render();
}

HRESULT CLensFlare::Ready_EffectPartObjects()
{
    CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tRectDesc{};
    tRectDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tRectDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
    tRectDesc.bUseTextureCom = true;
    tRectDesc.iTextureLevel = m_iPrototypeLevel;
    tRectDesc.bUseMaskCom = false;
    tRectDesc.bCustomShader = false;

    tRectDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGlow2");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Line"), &tRectDesc)))
        return E_FAIL;

    tRectDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle06");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Circle1"), &tRectDesc)))
        return E_FAIL;

    CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tMeshDesc{};
    tMeshDesc.iModelLevel = m_iPrototypeLevel;
    tMeshDesc.bUseDiffuseTexture = false;
    tMeshDesc.bUseUnknownTexture = true;
    tMeshDesc.bUseNormalTexture = false;
    tMeshDesc.bUseMRATexture = false;
    tMeshDesc.bUseTextureCom = true;
    tMeshDesc.iTextureLevel = m_iPrototypeLevel;
    tMeshDesc.bUseMaskCom = true;
    tMeshDesc.iMaskLevel = m_iPrototypeLevel;
    tMeshDesc.bCustomShader = false;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGradation");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon1"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Ring01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Ring1"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle01");
    tMeshDesc.bUseMaskCom = false;
    tMeshDesc.wstrMaskTag = L"";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Circle2"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle02");
    tMeshDesc.bUseMaskCom = true;
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon2"), &tMeshDesc)))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon3"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Ring01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Ring2"), &tMeshDesc)))
        return E_FAIL;

    tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
    tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle04");
    tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGradation");
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Circle3"), &tMeshDesc)))
        return E_FAIL;

    return S_OK;
}

void CLensFlare::Cache_LensElements()
{
    m_LensElements.clear();

    for (auto& [strTag, pPart] : m_EffestParts)
    {
        if (pPart == nullptr)
            continue;

        CTransform* pTransform = pPart->Get_Transform();

        if (pTransform == nullptr)
            continue;

        _float3 vAuthorPosition{};
        XMStoreFloat3(&vAuthorPosition, pTransform->Get_State(STATE::POSITION));

        LENS_ELEMENT Element{};
        Element.pPart = pPart;
        Element.vAuthorLocalPosition = vAuthorPosition;

        m_LensElements.emplace(strTag, Element);
    }

    m_bLensElementCacheReady = m_LensElements.size() == m_EffestParts.size();
}

_bool CLensFlare::Project_SourceToNDC(_float2* pOutSourceNDC) const
{
    if (pOutSourceNDC == nullptr)
        return false;

    const _matrix matView = XMLoadFloat4x4(
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC));

    const _matrix matProj = XMLoadFloat4x4(
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC));

    const _matrix matLensWorld = XMLoadFloat4x4(&m_CombinedWorldMatrix);
    const _vector vSourceWorld = XMVectorSetW(matLensWorld.r[3], 1.f);
    const _vector vSourceClip = XMVector4Transform(vSourceWorld, matView * matProj);
    const _float fClipW = XMVectorGetW(vSourceClip);

    if (fClipW <= Helper::fEpsilon)
        return false;

    const _float fInvW = 1.f / fClipW;

    pOutSourceNDC->x = XMVectorGetX(vSourceClip) * fInvW;
    pOutSourceNDC->y = XMVectorGetY(vSourceClip) * fInvW;

    return true;
}

_float CLensFlare::Calculate_AxisRatio(_float fAuthorZ) const
{
    const _float fRange = m_fAxisOppositeZ - m_fAxisSourceZ;

    if (fabsf(fRange) <= Helper::fEpsilon)
        return 0.f;

    const _float fNormalized = (fAuthorZ - m_fAxisSourceZ) / fRange;
    return fNormalized * m_fAxisExtent;
}

_float2 CLensFlare::Calculate_GhostNDC(const _float2& vSourceNDC, _float fAxisRatio) const
{
    const _float fAxisScale = 1.f - fAxisRatio;
    return { vSourceNDC.x * fAxisScale, vSourceNDC.y * fAxisScale };
}


_bool CLensFlare::Unproject_AtViewDepth(const _float2& vNDC, _float fViewDepth, _float3* pOutWorldPosition) const
{
    if (pOutWorldPosition == nullptr || fViewDepth <= Helper::fEpsilon)
        return false;

    const _matrix matInvProj = XMLoadFloat4x4(
        m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ));

    const _matrix matInvView = XMLoadFloat4x4(
        m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW));

    _vector vViewRay = XMVector3TransformCoord(
        XMVectorSet(vNDC.x, vNDC.y, 1.f, 1.f), matInvProj);

    const _float fRayZ = XMVectorGetZ(vViewRay);

    if (fabsf(fRayZ) <= Helper::fEpsilon)
        return false;

    vViewRay *= fViewDepth / fRayZ;
    vViewRay = XMVectorSetW(vViewRay, 1.f);

    const _vector vWorldPosition = XMVector3TransformCoord(vViewRay, matInvView);
    XMStoreFloat3(pOutWorldPosition, vWorldPosition);

    return true;
}

_bool CLensFlare::Update_LensFlarePlacement()
{
    if (m_bUseScreenAxis == false)
    {
        Restore_AuthorPlacement();
        return true;
    }

    m_bAuthorPlacementRestored = false;

    _float2 vSourceNDC{};

    if (Project_SourceToNDC(&vSourceNDC) == false)
        return false;

    if (fabsf(vSourceNDC.x) > m_fScreenCullMargin || fabsf(vSourceNDC.y) > m_fScreenCullMargin)
        return false;

    const _matrix matInvContainer = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_CombinedWorldMatrix));

    for (auto& [strTag, Element] : m_LensElements)
    {
        if (Element.pPart == nullptr)
            continue;

        CTransform* pPartTransform = Element.pPart->Get_Transform();

        if (pPartTransform == nullptr)
            continue;

        const _float fAxisRatio = Calculate_AxisRatio(Element.vAuthorLocalPosition.z);
        const _float2 vGhostNDC = Calculate_GhostNDC(vSourceNDC, fAxisRatio);

        _float3 vGhostWorld{};
        const _float fViewDepth = Element.vAuthorLocalPosition.z * m_fViewDepthScale;

        if (Unproject_AtViewDepth(vGhostNDC, fViewDepth, &vGhostWorld) == false)
            continue;

        const _vector vGhostLocal = XMVector3TransformCoord(XMLoadFloat3(&vGhostWorld), matInvContainer);
        pPartTransform->Set_State(STATE::POSITION, XMVectorSetW(vGhostLocal, 1.f));
    }

    return true;
}

void CLensFlare::Restore_AuthorPlacement()
{
    if (m_bAuthorPlacementRestored == true)
        return;

    if (m_bLensElementCacheReady == false)
        return;

    for (auto& [strTag, Element] : m_LensElements)
    {
        if (Element.pPart == nullptr)
            continue;

        CTransform* pPartTransform = Element.pPart->Get_Transform();

        if (pPartTransform == nullptr)
            continue;

        pPartTransform->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&Element.vAuthorLocalPosition), 1.f));
    }

    m_bAuthorPlacementRestored = true;
}

CLensFlare* CLensFlare::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLensFlare* pInstance = new CLensFlare(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CLensFlare");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLensFlare::Clone(void* pArg)
{
    CLensFlare* pInstance = new CLensFlare(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CLensFlare");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLensFlare::Free()
{
    __super::Free();
}