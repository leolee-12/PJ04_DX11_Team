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

	m_pUIAnimatorCom = Add_Component<CUIAnimatorCom>(
		TEXT("Com_UIAnimator"),
		CUIAnimatorCom::Create(m_pDevice, m_pContext));

	if (!m_pUIAnimatorCom || FAILED(m_pUIAnimatorCom->Initialize(nullptr)))
		return E_FAIL;

	Bind_UIAnimator();

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

	if (m_pUIAnimatorCom)
		m_pUIAnimatorCom->Update(fTimeDelta);

	__super::Update(fTimeDelta);

	if (m_pSpark && m_pSpark->Is_Active() && m_pSpark->Is_Finished())
	{
		m_pSpark->Stop();
		m_pSpark->Set_Active(false);
	}

	m_fVisibleElapsed += fTimeDelta;

	//if (!m_bFadeOut && m_fVisibleElapsed >= m_fVisibleDuration)
	//	Begin_PointStarFadeOut();

	if (m_bFadeOut && (!m_pUIAnimatorCom || !m_pUIAnimatorCom->Is_FadingAny()))
	{
		if (m_pSpark)
		{
			m_pSpark->Stop();
			m_pSpark->Set_Active(false);
		}

		if (m_pUIAnimatorCom)
			m_pUIAnimatorCom->Stop_AllBounces(true);

		m_bFadeOut = false;
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

	Bind_UIAnimator();

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
	Bind_UIAnimator();
}

HRESULT CUI_PointStar::Cache_Parts()
{
	m_pSpark = nullptr;
	m_pAmount = nullptr;

	auto FindPart = [this](const _wstring& strTag) -> CUIPartObject* 
		{
			auto it = m_UIPartObjects.find(strTag);
			if (it == m_UIPartObjects.end())
				return nullptr;

			return it->second;
		};

	m_pSpark = dynamic_cast<CUI_SpriteAnim*>(FindPart(L"Part_PointStar_SpriteAnim"));
	if (!m_pSpark)
		return E_FAIL;

	// 아직 텍스트 안넣었음
	m_pAmount = dynamic_cast<CUI_Text*>(FindPart(L"Part_PointStar_Amount"));

	if (!m_pAmount)
		return E_FAIL;

	return S_OK;
}

void CUI_PointStar::Bind_UIAnimator()
{
	if (m_pUIAnimatorCom)
		m_pUIAnimatorCom->Bind_Parts(m_UIPartObjects);
}

void CUI_PointStar::Play_PointStarBounce()
{
	const _float2 vDir = { 0.f, 1.f };

	if (m_pSpark)
	{
		m_pSpark->Set_Active(true);
		m_pSpark->Play(false);
	}

	if (!m_pUIAnimatorCom)
		return;

	//CUIAnimatorCom::UI_BOUNCE_DESC SparkBounce{};
	//SparkBounce.vDirection = vDir;
	//SparkBounce.fDistance = 10.f;
	//SparkBounce.fDuration = 0.28f;
	//SparkBounce.fWaveCount = 1.f;
	//SparkBounce.fDamping = 1.2f;
	//m_pUIAnimatorCom->Play_Bounce(L"Part_PointStar_SpriteAnim", SparkBounce);

	//CUIAnimatorCom::UI_BOUNCE_DESC IconBounce{};
	//IconBounce.vDirection = vDir;
	//IconBounce.fDistance = 7.f;
	//IconBounce.fDuration = 0.22f;
	//IconBounce.fWaveCount = 1.f;
	//IconBounce.fDamping = 1.f;
	//m_pUIAnimatorCom->Play_Bounce(L"Part_PointStar_Image", IconBounce);

	CUIAnimatorCom::UI_BOUNCE_DESC TextBounce{};
	TextBounce.vDirection = vDir;
	TextBounce.fDistance = 10.f;
	TextBounce.fDuration = 0.22f;
	TextBounce.fWaveCount = 1.f;
	TextBounce.fDamping = 1.f;
	m_pUIAnimatorCom->Play_Bounce(L"Part_PointStar_Amount", TextBounce);
}

void CUI_PointStar::Play_PointStarFadeIn()
{
	if (!m_pUIAnimatorCom)
		return;

	m_pUIAnimatorCom->Stop_AllFades(true);

	CUIAnimatorCom::UI_FADE_DESC FadeIn{};
	FadeIn.fFromAlpha = 0.f;
	FadeIn.fToAlpha = -1.f;
	FadeIn.fDuration = m_fFadeInDuration;
	FadeIn.bRestoreOnFinish = false;

	m_pUIAnimatorCom->Play_FadeAll(FadeIn);
}

void CUI_PointStar::Begin_PointStarFadeOut()
{
	if (m_bFadeOut)
		return;

	m_bFadeOut = true;

	if (!m_pUIAnimatorCom)
		return;

	m_pUIAnimatorCom->Stop_AllBounces(true);

	CUIAnimatorCom::UI_FADE_DESC FadeOut{};
	FadeOut.fFromAlpha = -1.f;
	FadeOut.fToAlpha = 0.f;
	FadeOut.fDuration = m_fFadeOutDuration;
	FadeOut.bRestoreOnFinish = true;

	m_pUIAnimatorCom->Play_FadeAll(FadeOut);
}

void CUI_PointStar::On_PointStarGained(const KIRBY_POINTSTAR_GAINED_DESC* pDesc)
{
	// UI Active
	const _bool bWasActive = Is_Active();
	const _bool bWasFadeOut = m_bFadeOut;

	Set_Active(true);
	m_fVisibleElapsed = 0.f;
	m_bFadeOut = false;

	if (!bWasActive)
	{
		Play_PointStarFadeIn();
	}
	else if (bWasFadeOut && m_pUIAnimatorCom)
	{
		m_pUIAnimatorCom->Stop_AllFades(true);
	}

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
