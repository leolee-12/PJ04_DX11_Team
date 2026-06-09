#include "UI_PointStar.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_SpriteAnim.h"
#include "UI_Effect.h"

CUI_PointStar::CUI_PointStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIContainerObject{ pDevice, pContext }
{
}

CUI_PointStar::CUI_PointStar(const CUI_PointStar& Prototype)
	: CUIContainerObject(Prototype)
{
}

HRESULT CUI_PointStar::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_PointStar::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CUI_PointStar::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CUI_PointStar::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);

	if (m_pSpark && m_pSpark->Is_Active() && m_pSpark->Is_Finished())
	{
		m_pSpark->Stop();
		m_pSpark->Set_Active(false);
	}

	m_fVisibleElapsed += fTimeDelta;

	if (m_fVisibleElapsed >= m_fVisibleDuration)
	{
		if (m_pSpark)
		{
			m_pSpark->Stop();
			m_pSpark->Set_Active(false);
		}

		if (m_pIcon)
			m_pIcon->Stop_Bounce(true);

		if (m_pAmount)
			m_pAmount->Stop_Bounce(true);

		Set_Active(false);
	}
}

void CUI_PointStar::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CUI_PointStar::Ready_Events()
{
	if (FAILED(__super::Ready_Events()))
		return E_FAIL;

	Subscribe_Event(EventTag::Kirby_PointStarGained,
		[this](void* pData)
		{
			auto* pDesc = static_cast<KIRBY_POINTSTAR_GAINED_DESC*>(pData);
			On_PointStarGained(pDesc);
		});

	return S_OK;
}

void CUI_PointStar::On_Deserialized()
{
	if (FAILED(Cache_Parts()))
		return;

	if (m_pSpark)
	{
		m_pSpark->Stop();
		m_pSpark->Set_Active(false);
	}

	Refresh_AmountText();
}

void CUI_PointStar::On_UIPartsChanged()
{
	Cache_Parts();
}

HRESULT CUI_PointStar::Cache_Parts()
{
	m_pIcon = nullptr;
	m_pSpark = nullptr;
	m_pAmount = nullptr;

	auto FindPart = [this](const _wstring& strTag) -> CUIPartObject* 
		{
			auto it = m_UIPartObjects.find(strTag);
			if (it == m_UIPartObjects.end())
				return nullptr;

			return it->second;
		};

	m_pIcon = dynamic_cast<CUI_Image*>(FindPart(L"Part_PointStar_Image"));
	if (!m_pIcon)
		return E_FAIL;

	m_pSpark = dynamic_cast<CUI_SpriteAnim*>(FindPart(L"Part_PointStar_SpriteAnim"));
	if (!m_pSpark)
		return E_FAIL;

	// 아직 텍스트 안넣었음
	m_pAmount = dynamic_cast<CUI_Text*>(FindPart(L"Part_PointStar_Amount"));

	if (!m_pAmount)
		return E_FAIL;

	return S_OK;
}

void CUI_PointStar::Play_PointStarBounce()
{
	const _float2 vDir = { 0.f, 1.f };

	if (m_pSpark)
	{
		m_pSpark->Set_Active(true);
		m_pSpark->Play(false);
		m_pSpark->Play_Bounce(vDir, 10.f, 0.28f, 1.f, 1.2f);
	}

	if (m_pIcon)
		m_pIcon->Play_Bounce(vDir, 7.f, 0.22f, 1.f, 1.f);

	if (m_pAmount)
		m_pAmount->Play_Bounce(vDir, 6.f, 0.22f, 1.f, 1.f);
}

void CUI_PointStar::On_PointStarGained(const KIRBY_POINTSTAR_GAINED_DESC* pDesc)
{
	// UI Active
	m_bActive = true;
	m_fVisibleElapsed = 0.f;

	const _uint iAmount = pDesc ? pDesc->iAmount : 1;

	m_iStarCount += iAmount;

	Refresh_AmountText();
	Play_PointStarBounce();
}

void CUI_PointStar::Refresh_AmountText()
{
	if (!m_pAmount)
		return;

	m_pAmount->Set_Text(to_wstring(m_iStarCount));
}

CUI_PointStar* CUI_PointStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_PointStar* pInstance = new CUI_PointStar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_PointStar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_PointStar::Clone(void* pArg)
{
	CUI_PointStar* pInstance = new CUI_PointStar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_PointStar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_PointStar::Free()
{
	__super::Free();
}
