#include "Cage_WaddleDee.h"
#include "Animator.h"
#include "GameInstance.h"

namespace
{
    struct DEE_CLIPS
    {
        const _char* szCaged;     // 갇혀 있는 동안 루프
        const _char* szRescued;   // 구출 후 춤
    };

    constexpr const _char* s_RescuedClips[ETOUI(CCage_WaddleDee::DEE_POS::END)] =
    {
        "DeeFront_Dance",
        "DeeLeft_Dance",
        "DeeRight_Dance",
    };

    constexpr _float g_fDanceDuration = { 4.f };

    constexpr const _char* s_CagedWaits[] =
    {
        "CageSWaitA", "CageSWaitB", "CageSWaitC", "CageSWaitD",
    };
    constexpr _uint g_iNumCagedWaits = static_cast<_uint>(std::size(s_CagedWaits));
}

CCage_WaddleDee::CCage_WaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart{ pDevice, pContext }
{
}

CCage_WaddleDee::CCage_WaddleDee(const CCage_WaddleDee& Prototype)
    : CMonsterPart(Prototype)
{
}

HRESULT CCage_WaddleDee::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCage_WaddleDee::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg)))   // pParentMatrix + pSocketBoneMatrix는 베이스가 받는다
        return E_FAIL;

    if (auto pDesc = static_cast<WADDLEDEE_DESC*>(pArg))
    {
        m_ePos = pDesc->ePos;
        m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->InitialLocal));
    }

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_iCagedClipIdx = static_cast<_uint>(m_pGameInstance_Proxy->RandomInt(0, g_iNumCagedWaits - 1));
    m_pAnimatorCom->Play(s_CagedWaits[m_iCagedClipIdx], false, false);

    return S_OK;
}

void CCage_WaddleDee::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    __super::Update(fTimeDelta);   // 애니메이터 갱신

    if (STATE::CAGED == m_eState && m_pAnimatorCom->Is_Finished())
        Play_RandomCageWait();

    if (STATE::RESCUED == m_eState)
    {
        m_fDanceTimer += fTimeDelta;
        if (m_fDanceTimer > g_fDanceDuration)
        {
            m_eState = STATE::DONE;
            Set_Active(false);     // 파츠만 끈다. 케이지 정리는 컨테이너가 결정
        }
    }
}

HRESULT CCage_WaddleDee::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes =
        static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPass = 0;
        if (i == 0)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_EyeTexture", i, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_EyeMaskTexture", i, MTEX_TYPE::UNKNOWN, 2)))
                return E_FAIL;
        }
        else
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPass = 1;
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CCage_WaddleDee::Rescue()
{
    if (STATE::CAGED != m_eState)
        return;

    if (m_pSocketBoneMatrix)
    {
        // 이 순간의 본 행렬을 로컬에 흡수 -> 본 연결이 끊겨도 위치가 튀지 않고
        // 이후엔 로컬' x 케이지월드로만 합성된다
        _matrix Baked = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())
            * XMLoadFloat4x4(m_pSocketBoneMatrix);
        m_pTransformCom->Set_WorldMatrix(Baked);
        m_pSocketBoneMatrix = nullptr;
    }

    m_eState = STATE::RESCUED;
    m_fDanceTimer = 0.f;
    m_pAnimatorCom->Play(s_RescuedClips[ETOUI(m_ePos)], true, true);
}

HRESULT CCage_WaddleDee::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_WaddleDee;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = true;

    return Ready_MeshPart(t);
}

void CCage_WaddleDee::Play_RandomCageWait()
{
    _uint iNext = static_cast<_uint>(m_pGameInstance_Proxy->RandomInt(0, g_iNumCagedWaits - 2));
    if (iNext >= m_iCagedClipIdx)
        ++iNext;

    m_iCagedClipIdx = iNext;
    m_pAnimatorCom->Play(s_CagedWaits[iNext], false, false);
}

CCage_WaddleDee* CCage_WaddleDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCage_WaddleDee* pInstance = new CCage_WaddleDee(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCage_WaddleDee");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CCage_WaddleDee::Clone(void* pArg)
{
    CCage_WaddleDee* pInstance = new CCage_WaddleDee(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCage_WaddleDee");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCage_WaddleDee::Free()
{
    __super::Free();
}