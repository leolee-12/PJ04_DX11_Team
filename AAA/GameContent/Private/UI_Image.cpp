#include "UI_Image.h"
#include "GameInstance_Proxy.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer_Rect.h"

CUI_Image::CUI_Image(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIPartObject{ pDevice, pContext }
	, m_vColor{ 1.f, 1.f, 1.f, 1.f }
	, m_fAlpha{ 1.f }
	, m_iTextureLevel {ETOUI(LEVEL::STATIC)}
{
}

CUI_Image::CUI_Image(const CUI_Image& Prototype)
	: CUIPartObject(Prototype)
	, m_vColor{ Prototype.m_vColor }
	, m_fAlpha{ Prototype.m_fAlpha }
	, m_iTextureLevel{ Prototype.m_iTextureLevel }
	, m_strTextureProtoTag { Prototype.m_strTextureProtoTag }
{
}

HRESULT CUI_Image::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_Image::Initialize(void* pArg)
{
	UI_IMAGE_DESC Default{};
	UI_IMAGE_DESC* pDesc = pArg ? static_cast<UI_IMAGE_DESC*>(pArg) : &Default;

	if (pDesc->szTextureProtoTag)
	{
		m_iTextureLevel = static_cast<int>(pDesc->iTextureLevel);
		m_strTextureProtoTag = pDesc->szTextureProtoTag;
	}

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	m_eRenderLayer = static_cast<RENDERUIID>(pDesc->iRenderLayer);

	// ortho  디자인 좌표계
	m_pTransformCom->Set_Scale(pDesc->vSize.x, pDesc->vSize.y, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, 1.f, 1.f));			// 카메라 z값이 0.1이므로 0으로 설정하면 화면에 안보임

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	return S_OK;
}

void CUI_Image::Priority_Update(_float fTimeDelta)
{
}

void CUI_Image::Update(_float fTimeDetla)
{
	__super::Update(fTimeDetla);
}

void CUI_Image::Late_Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);		 // UI 전용 큐 ->  z-order 정렬됨
}

HRESULT CUI_Image::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;
	return S_OK;
}

void CUI_Image::Deserialize_Internal(const json& j)
{
	__super::Deserialize_Internal(j);

	if (m_strTextureProtoTag.empty())
		return;

	if (m_pTextureCom)
		return;

	m_pTextureCom = Add_Component<CTexture>(
		static_cast<_uint>(m_iTextureLevel),
		m_strTextureProtoTag,
		TEXT("Com_Texture"));
}

HRESULT CUI_Image::Set_Texture(_int iLevel, const _wstring& strProtoTag)
{
	if (strProtoTag.empty())
		return E_FAIL;

	// try_emplace 는 덮어쓰지 않으므로, 기존 Com_Texture 를 직접 해제 후 제거
	auto iter = m_Components.find(TEXT("Com_Texture"));
	if (iter != m_Components.end())
	{
		Safe_Release(iter->second);
		m_Components.erase(iter);
		m_pTextureCom = nullptr;
	}

	m_iTextureLevel = iLevel;
	m_strTextureProtoTag = strProtoTag;

	m_pTextureCom = Add_Component<CTexture>(
		static_cast<_uint>(m_iTextureLevel),
		m_strTextureProtoTag,
		TEXT("Com_Texture"));

	return m_pTextureCom ? S_OK : E_FAIL;
}

CUI_Image* CUI_Image::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Image* pInstance = new CUI_Image(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Image");
		Safe_Release(pInstance);
	}
	return pInstance;
}


CGameObject* CUI_Image::Clone(void* pArg)
{
	CUI_Image* pInstance = new CUI_Image(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Image");
		Safe_Release(pInstance);
	}
	return pInstance;
}

HRESULT CUI_Image::Ready_Components(UI_IMAGE_DESC* pDesc)
{
	m_pShaderCom = Add_Component<CShader>(
		Shader_UI.iLevelID,
		Shader_UI.szProtoTag,
		TEXT("Com_Shader"));

	if (!m_pShaderCom)
		return E_FAIL;

	m_pVIBufferCom = Add_Component<CVIBuffer_Rect>(
		VI_Rect.iLevelID,
		VI_Rect.szProtoTag,
		TEXT("Com_Model"));

	if (!m_pVIBufferCom)
		return E_FAIL;

	if (!m_strTextureProtoTag.empty())
	{
		m_pTextureCom = Add_Component<CTexture>(
			static_cast<_uint>(m_iTextureLevel),
			m_strTextureProtoTag,
			TEXT("Com_Texture"));

		if (!m_pTextureCom)
			return E_FAIL;
	}

	return S_OK;
}

HRESULT	CUI_Image::Bind_ShaderResources()
{
	if (m_pParentMatrix)
	{
		Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
			return E_FAIL;
	}
	else if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
		return E_FAIL;

	if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW, PROJ_TYPE::ORTHO)))
		return E_FAIL;
	if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ, PROJ_TYPE::ORTHO)))
		return E_FAIL;

	if (m_pTextureCom)
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTexIndex)))
			return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
		return E_FAIL;

	return S_OK;
}


void CUI_Image::Free()
{
	__super::Free();
}
