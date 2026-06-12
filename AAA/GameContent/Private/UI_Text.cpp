#include "UI_Text.h"
#include "GameInstance_Proxy.h"
#include "GameContent_const.h"

CUI_Text::CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIPartObject(pDevice, pContext)
	, m_vColor{ 1.f, 1.f, 1.f, 1.f }
	, m_fFontScale{ 1.f }
	, m_iAlign{ 1 }
	, m_fRotation{ 0.f }
{
}

CUI_Text::CUI_Text(const CUI_Text& Prototype)
	: CUIPartObject(Prototype)
	, m_vColor{ Prototype.m_vColor }
	, m_strText{ Prototype.m_strText }
	, m_strFontTag{ Prototype.m_strFontTag }
	, m_fFontScale{ Prototype.m_fFontScale }
	, m_iAlign{ Prototype.m_iAlign }
	, m_fRotation{ Prototype.m_fRotation }
{
}

HRESULT CUI_Text::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_Text::Initialize(void* pArg)
{
	UI_TEXT_DESC Default{};
	UI_TEXT_DESC* pDesc = pArg ? static_cast<UI_TEXT_DESC*>(pArg) : &Default;

	if (pDesc->szText)
		m_strText = pDesc->szText;

	if (pDesc->szFontTag)
		m_strFontTag = pDesc->szFontTag;

	m_vColor = pDesc->vColor;
	m_fFontScale = pDesc->fFontScale;
	m_iAlign = pDesc->iAlign;
	m_fRotation = pDesc->fRotation;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_eRenderLayer = static_cast<RENDERUIID>(pDesc->iRenderLayer);

	m_pTransformCom->Set_Scale(pDesc->vSize.x, pDesc->vSize.y, 1.f);

	// Transform의 위치만 사용
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, 1.f, 1.f));

	return S_OK;
}

void CUI_Text::Priority_Update(_float fTimeDelta)
{
}

void CUI_Text::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
	Bake_Text();
	m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);
}

HRESULT CUI_Text::Render()
{
	if (!m_pTextSRV) return S_OK;

	if (m_pParentMatrix)
	{
		Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix))) return E_FAIL;
	}
	else if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
		return E_FAIL;

	if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW, PROJ_TYPE::ORTHO))) return E_FAIL;
	if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ, PROJ_TYPE::ORTHO))) return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_SRV("g_Texture", m_pTextSRV))) return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4)))) return E_FAIL;
	_float fAlpha = 1.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(_float)))) return E_FAIL;
	_float4 vUV = { 1.f, 1.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVTransform", &vUV, sizeof(_float4)))) return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0))) return E_FAIL;          // 이미지랑 동일 텍스처 패스
	if (FAILED(m_pVIBufferCom->Bind_Resources())) return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render())) return E_FAIL;
	return S_OK;
}

_float2 CUI_Text::Get_TextSize() const
{
	if (m_strText.empty() || m_strFontTag.empty())
		return _float2(0.f, 0.f);

	_float2 v = m_pGameInstance_Proxy->Measure_Text(m_strFontTag, m_strText.c_str());

	return _float2(v.x * m_fFontScale, v.y * m_fFontScale);
}

HRESULT CUI_Text::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(Shader_UI.iLevelID, Shader_UI.szProtoTag, TEXT("Com_Shader"));
	if (!m_pShaderCom) return E_FAIL;
	m_pVIBufferCom = Add_Component<CVIBuffer_Rect>(VI_Rect.iLevelID, VI_Rect.szProtoTag, TEXT("Com_VIBuffer"));
	if (!m_pVIBufferCom) return E_FAIL;
	return S_OK;
}

HRESULT CUI_Text::Ensure_RT(_uint iW, _uint iH)
{
	if (m_pTextTex && iW == m_iTexW && iH == m_iTexH)
		return S_OK;

	Safe_Release(m_pTextSRV);
	Safe_Release(m_pTextRTV);
	Safe_Release(m_pTextTex);
	m_iTexW = iW; m_iTexH = iH;

	D3D11_TEXTURE2D_DESC td{};
	td.Width = iW; td.Height = iH; td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(m_pDevice->CreateTexture2D(&td, nullptr, &m_pTextTex)))                 return E_FAIL;
	if (FAILED(m_pDevice->CreateRenderTargetView(m_pTextTex, nullptr, &m_pTextRTV)))   return E_FAIL;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTextTex, nullptr, &m_pTextSRV))) return E_FAIL;
	return S_OK;
}

void CUI_Text::Bake_Text()
{
	// RT 크기 = 내가 세팅한 박스(transform scale) 픽셀, 고정
	_float3 s = m_pTransformCom->Get_Scaled();
	_uint w = max(1u, (_uint)ceilf(s.x));
	_uint h = max(1u, (_uint)ceilf(s.y));

	const _bool bSizeChanged = (w != m_iTexW || h != m_iTexH);
	if (!m_bTextDirty && !bSizeChanged) return;     // 텍스트도 안 바뀌고 박스도 그대로면 스킵

	if (FAILED(Ensure_RT(w, h))) return;

	ID3D11RenderTargetView* pOldRTV = nullptr;
	ID3D11DepthStencilView* pOldDSV = nullptr;
	m_pContext->OMGetRenderTargets(1, &pOldRTV, &pOldDSV);
	UINT nVP = 1; D3D11_VIEWPORT oldVP{};
	m_pContext->RSGetViewports(&nVP, &oldVP);

	const float clear[4] = { 0.f, 0.f, 0.f, 0.f };
	m_pContext->ClearRenderTargetView(m_pTextRTV, clear);   // 빈 텍스트면 여기서 끝(투명)
	m_pContext->OMSetRenderTargets(1, &m_pTextRTV, nullptr);
	D3D11_VIEWPORT vp{ 0.f, 0.f, (float)m_iTexW, (float)m_iTexH, 0.f, 1.f };
	m_pContext->RSSetViewports(1, &vp);

	if (!m_strText.empty() && !m_strFontTag.empty())
	{
		m_pGameInstance_Proxy->Draw_Text_Raw(
			m_strFontTag, m_strText.c_str(),
			_float2(0.f, 0.f),                        
			XMVectorSet(1.f, 1.f, 1.f, 1.f),
			_float2(m_fFontScale, m_fFontScale),
			TEXT_ALIGN::LEFT);
	}

	m_pContext->OMSetRenderTargets(1, &pOldRTV, pOldDSV);
	m_pContext->RSSetViewports(1, &oldVP);
	Safe_Release(pOldRTV);
	Safe_Release(pOldDSV);

	m_bTextDirty = false;
}

CUI_Text* CUI_Text::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Text* pInstance = new CUI_Text(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Text");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Text::Clone(void* pArg)
{
	CUI_Text* pInstance = new CUI_Text(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Text");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Text::Free()
{
	Safe_Release(m_pTextSRV);
	Safe_Release(m_pTextRTV);
	Safe_Release(m_pTextTex);
	__super::Free();
}
