#include "BrontoBurt_Body.h"
#include "Animator.h"
#include "GameInstance.h"

CBrontoBurt_Body::CBrontoBurt_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonsterPart{ pDevice, pContext }
{
}

CBrontoBurt_Body::CBrontoBurt_Body(const CBrontoBurt_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CBrontoBurt_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CBrontoBurt_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<BRONTOBURT_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Fly", true, true);

	return S_OK;
}

void CBrontoBurt_Body::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CBrontoBurt_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes =
        static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_UnknownTexture", m_iEyeIndex)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (m_pAnimatorCom)
        {
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(0)))   // eye pass
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBrontoBurt_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;
    t.szModelProtoTag =
        TEXT("Prototype_Component_Model_BrontoBurt_Body");

    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

    // 눈 표정 텍스처 배열
    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext,
            L"../../Resources/CHJ/Monster/BrontoBurt/Face.%02d.dds", EYE_COUNT));
    if (nullptr == m_pEyeTextureCom)
        return E_FAIL;

	return S_OK;
}

CBrontoBurt_Body* CBrontoBurt_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBrontoBurt_Body* pInstance = new CBrontoBurt_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBrontoBurt_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBrontoBurt_Body::Clone(void* pArg)
{
    CBrontoBurt_Body* pInstance = new CBrontoBurt_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBrontoBurt_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBrontoBurt_Body::Free()
{
    __super::Free();
}
