#include "UI_Text.h"
#include "GameInstance_Proxy.h"

CUI_Text::CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIPartObject { pDevice, pContext }
	, m_vColor{ 1.f, 1.f, 1.f, 1.f }
	, m_fFontScale{ 1.f }
	, m_iAlign{ 1 }
	, m_fRotation{ 0.f }
{
}

CUI_Text::CUI_Text(const CUI_Text& Prototype)
	: CUIPartObject { Prototype }
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
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);
}

HRESULT CUI_Text::Render()
{
	if (m_strText.empty() || m_strFontTag.empty())
		return S_OK;

	_float4x4 box;
	if (m_pParentMatrix)
	{
		Compute_CombinedWorldMatrix(
			XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
		box = m_CombinedWorldMatrix;
	}
	else
		box = *m_pTransformCom->Get_WorldMatrixPtr();

	_float2 vBoxCenter = { box._41, box._42 };
	_float  fBoxW = sqrtf(box._11 * box._11 +
		box._12 * box._12 + box._13 * box._13);   // row0 길이 = 박스 폭

	// 박스 안 정렬: 앵커를 좌/중/우 모서리로
	_float fAnchorX = vBoxCenter.x;                          // Center
	if (m_iAlign == 0)      fAnchorX = vBoxCenter.x - fBoxW * 0.5f;  // Left
	else if (m_iAlign == 2) fAnchorX = vBoxCenter.x + fBoxW * 0.5f;  // Right

	_float2 vPos = { fAnchorX, vBoxCenter.y };

	m_pGameInstance_Proxy->Draw_Text(
		m_strFontTag, m_strText.c_str(), vPos,
		XMLoadFloat4(&m_vColor), m_fRotation,
		_float2(m_fFontScale, m_fFontScale),
		static_cast<TEXT_ALIGN>(m_iAlign));

	return S_OK;
}

_float2 CUI_Text::Get_TextSize() const
{
	if (m_strText.empty() || m_strFontTag.empty())
		return _float2(0.f, 0.f);

	_float2 v = m_pGameInstance_Proxy->Measure_Text(m_strFontTag, m_strText.c_str());

	return _float2(v.x * m_fFontScale, v.y * m_fFontScale);
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
	__super::Free();
}
