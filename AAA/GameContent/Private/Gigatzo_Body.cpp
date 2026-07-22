#include "Gigatzo_Body.h"
#include "GameInstance.h"

CGigatzo_Body::CGigatzo_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CGigatzo_Body::CGigatzo_Body(const CGigatzo_Body& Prototype)
	: CMonsterPart(Prototype)
{
}

HRESULT CGigatzo_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CGigatzo_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<GIGATZO_BODY_DESC*>(pArg);
	
	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CGigatzo_Body::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CGigatzo_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", 0, MTEX_TYPE::DIFFUSE, 0)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", 0, MTEX_TYPE::NORMALS, 0)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", 0, MTEX_TYPE::METALNESS, 0)))
        return E_FAIL;
    if (m_pAnimatorCom)
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0)))
            return E_FAIL;
    if (FAILED(m_pShaderCom->Begin(1)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Render(0)))
        return E_FAIL;
    return S_OK;
}

HRESULT CGigatzo_Body::Render_Shadow()
{
    if (!m_bActive || nullptr == m_pModelCom)
        return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix",
        m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix",
        m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
        return E_FAIL;

    if (m_pAnimatorCom)
        m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0);
    if (FAILED(m_pShaderCom->Begin(m_iShadowPassIdx)))
        return E_FAIL;
    if (FAILED(m_pModelCom->Render(0)))
        return E_FAIL;
    return S_OK;
}

HRESULT CGigatzo_Body::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_Monster;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_Gigatzo_Body");
	t.szAnimEventFile = TEXT("../../Resources/CHJ/Monster/Gigatzo/Body/Gigatzo_AnimEvents.json");

	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	return S_OK;
}

CGigatzo_Body* CGigatzo_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGigatzo_Body* pInstance = new CGigatzo_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGigatzo_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGigatzo_Body::Clone(void* pArg)
{
	CGigatzo_Body* pInstance = new CGigatzo_Body(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGigatzo_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGigatzo_Body::Free()
{
	__super::Free();
}
