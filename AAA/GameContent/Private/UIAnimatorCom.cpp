#include "UIAnimatorCom.h"
#include "UIPartObject.h"
#include "Transform.h"

CUIAnimatorCom::CUIAnimatorCom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CUIAnimatorCom::CUIAnimatorCom(const CUIAnimatorCom& Prototype)
	: CComponent( Prototype )
{
}

HRESULT CUIAnimatorCom::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUIAnimatorCom::Initialize(void* pArg)
{
	return __super::Initialize(pArg);
}

void CUIAnimatorCom::Update(_float fTimeDelta)
{
	for (auto iter = m_BounceRuntimes.begin(); iter != m_BounceRuntimes.end();)
	{
		UI_BOUNCE_RUNTIME& Runtime = iter->second;
		CUIPartObject* pTarget = Runtime.pTarget;

		if (!pTarget || !pTarget->Get_Transform() || Runtime.Desc.fDuration <= FLT_EPSILON)
		{
			iter = m_BounceRuntimes.erase(iter);
			continue;
		}

		Runtime.fElapsed += fTimeDelta;

		_float fT = Runtime.fElapsed / Runtime.Desc.fDuration;
		if (fT >= 1.f)
		{
			if (Runtime.Desc.bRestoreOnFinish)
				Restore_BounceBase(Runtime);

			iter = m_BounceRuntimes.erase(iter);
			continue;
		}

		fT = max(0.f, min(1.f, fT));

		const _float fWave =
			sinf(XM_PI * Runtime.Desc.fWaveCount * fT);

		const _float fEnvelope =
			(Runtime.Desc.fDamping <= 0.f)
			? 1.f
			: powf(1.f - fT, Runtime.Desc.fDamping);

		const _float fOffset =
			Runtime.Desc.fDistance * fWave * fEnvelope;

		const _float2 vPos =
		{
			Runtime.vBasePos.x + Runtime.Desc.vDirection.x * fOffset,
			Runtime.vBasePos.y + Runtime.Desc.vDirection.y * fOffset
		};

		pTarget->Get_Transform()->Set_State(
			STATE::POSITION,
			XMVectorSet(vPos.x, vPos.y, Runtime.fBaseZ, 1.f));

		++iter;
	}

	for (auto iter = m_FadeRuntimes.begin(); iter != m_FadeRuntimes.end();)
	{
		UI_FADE_RUNTIME& Runtime = iter->second;
		CUIPartObject* pTarget = Runtime.pTarget;

		if (!pTarget || Runtime.Desc.fDuration <= FLT_EPSILON)
		{
			iter = m_FadeRuntimes.erase(iter);
			continue;
		}

		Runtime.fElapsed += fTimeDelta;

		const _float fDelay = max(0.f, Runtime.Desc.fDelay);
		const _float fFadeElapsed = Runtime.fElapsed - fDelay;

		if (fFadeElapsed <= 0.f)
		{
			Try_SetAlpha(pTarget, Runtime.fFromAlpha);
			++iter;
			continue;
		}

		_float fT = fFadeElapsed / Runtime.Desc.fDuration;
		if (fT >= 1.f)
		{
			Try_SetAlpha(pTarget, Runtime.Desc.fToAlpha);

			if (Runtime.Desc.bRestoreOnFinish)
				Restore_FadeBase(Runtime);

			iter = m_FadeRuntimes.erase(iter);
			continue;
		}

		fT = max(0.f, min(1.f, fT));

		const _float fAlpha =
			Runtime.fFromAlpha + (Runtime.Desc.fToAlpha - Runtime.fFromAlpha) * fT;

		Try_SetAlpha(pTarget, fAlpha);

		++iter;
	}
}

void CUIAnimatorCom::Bind_Parts(const unordered_map<_wstring, CUIPartObject*>& Parts)
{
	m_Parts = Parts;
	m_BounceRuntimes.clear();
	m_FadeRuntimes.clear();
}

void CUIAnimatorCom::Clear_Parts()
{
	m_Parts.clear();
	m_BounceRuntimes.clear();
	m_FadeRuntimes.clear();
}

HRESULT CUIAnimatorCom::Play_Bounce(const _wstring& strPartTag, const UI_BOUNCE_DESC& Desc)
{
	return Play_Bounce(Find_Part(strPartTag), Desc);
}

HRESULT CUIAnimatorCom::Play_Bounce(CUIPartObject* pPart, const UI_BOUNCE_DESC& Desc)
{
	if (!pPart || !pPart->Get_Transform())
		return E_FAIL;

	if (Desc.fDistance <= 0.f || Desc.fDuration <= FLT_EPSILON)
		return E_FAIL;

	_vector vDir = XMLoadFloat2(&Desc.vDirection);
	if (XMVectorGetX(XMVector2LengthSq(vDir)) <= FLT_EPSILON)
		return E_FAIL;

	vDir = XMVector2Normalize(vDir);

	Stop_Bounce(pPart, true);

	_vector vPos = pPart->Get_Transform()->Get_State(STATE::POSITION);

	UI_BOUNCE_RUNTIME Runtime{};
	Runtime.pTarget = pPart;
	Runtime.vBasePos =
	{
		XMVectorGetX(vPos),
		XMVectorGetY(vPos)
	};
	Runtime.fBaseZ = XMVectorGetZ(vPos);
	Runtime.Desc = Desc;
	Runtime.Desc.fWaveCount = max(0.1f, Desc.fWaveCount);
	Runtime.Desc.fDamping = max(0.f, Desc.fDamping);
	Runtime.fElapsed = 0.f;

	XMStoreFloat2(&Runtime.Desc.vDirection, vDir);

	m_BounceRuntimes.emplace(pPart, Runtime);

	return S_OK;
}

void CUIAnimatorCom::Stop_Bounce(const _wstring& strPartTag, _bool bRestoreBase)
{
	Stop_Bounce(Find_Part(strPartTag), bRestoreBase);
}

void CUIAnimatorCom::Stop_Bounce(CUIPartObject* pPart, _bool bRestoreBase)
{
	if (!pPart)
		return;

	auto iter = m_BounceRuntimes.find(pPart);
	if (iter == m_BounceRuntimes.end())
		return;

	if (bRestoreBase)
		Restore_BounceBase(iter->second);

	m_BounceRuntimes.erase(iter);
}

void CUIAnimatorCom::Stop_AllBounces(_bool bRestoreBase)
{
	if (bRestoreBase)
	{
		for (auto& Pair : m_BounceRuntimes)
			Restore_BounceBase(Pair.second);
	}

	m_BounceRuntimes.clear();
}

_bool CUIAnimatorCom::Is_Bouncing(const _wstring& strPartTag) const
{
	return Is_Bouncing(Find_Part(strPartTag));
}

_bool CUIAnimatorCom::Is_Bouncing(CUIPartObject* pPart) const
{
	return pPart && m_BounceRuntimes.find(pPart) != m_BounceRuntimes.end();
}

HRESULT CUIAnimatorCom::Play_Fade(const _wstring& strPartTag, const UI_FADE_DESC& Desc)
{
	return Play_Fade(Find_Part(strPartTag), Desc);
}

HRESULT CUIAnimatorCom::Play_Fade(CUIPartObject* pPart, const UI_FADE_DESC& Desc)
{
	if (!pPart || Desc.fDuration <= FLT_EPSILON)
		return E_FAIL;

	_float fBaseAlpha = 1.f;
	if (!Try_GetAlpha(pPart, &fBaseAlpha))
		return E_FAIL;

	Stop_Fade(pPart, false);

	UI_FADE_RUNTIME Runtime{};
	Runtime.pTarget = pPart;
	Runtime.Desc = Desc;
	Runtime.Desc.fDelay = max(0.f, Desc.fDelay);
	Runtime.fBaseAlpha = max(0.f, min(1.f, fBaseAlpha));
	Runtime.fFromAlpha = (Desc.fFromAlpha < 0.f)
		? Runtime.fBaseAlpha
		: max(0.f, min(1.f, Desc.fFromAlpha));
	Runtime.Desc.fToAlpha = (Desc.fToAlpha < 0.f)
		? Runtime.fBaseAlpha
		: max(0.f, min(1.f, Desc.fToAlpha));
	Runtime.fElapsed = 0.f;

	if (!Try_SetAlpha(pPart, Runtime.fFromAlpha))
		return E_FAIL;

	m_FadeRuntimes.emplace(pPart, Runtime);

	return S_OK;
}

void CUIAnimatorCom::Play_FadeAll(const UI_FADE_DESC& Desc)
{
	for (const auto& Pair : m_Parts)
		Play_Fade(Pair.second, Desc);
}

void CUIAnimatorCom::Stop_Fade(const _wstring& strPartTag, _bool bRestoreBase)
{
	Stop_Fade(Find_Part(strPartTag), bRestoreBase);
}

void CUIAnimatorCom::Stop_Fade(CUIPartObject* pPart, _bool bRestoreBase)
{
	if (!pPart)
		return;

	auto iter = m_FadeRuntimes.find(pPart);
	if (iter == m_FadeRuntimes.end())
		return;

	if (bRestoreBase)
		Restore_FadeBase(iter->second);

	m_FadeRuntimes.erase(iter);
}

void CUIAnimatorCom::Stop_AllFades(_bool bRestoreBase)
{
	if (bRestoreBase)
	{
		for (auto& Pair : m_FadeRuntimes)
			Restore_FadeBase(Pair.second);
	}

	m_FadeRuntimes.clear();
}

_bool CUIAnimatorCom::Is_Fading(const _wstring& strPartTag) const
{
	return Is_Fading(Find_Part(strPartTag));
}

_bool CUIAnimatorCom::Is_Fading(CUIPartObject* pPart) const
{
	return pPart && m_FadeRuntimes.find(pPart) != m_FadeRuntimes.end();
}

_bool CUIAnimatorCom::Is_FadingAny() const
{
	return !m_FadeRuntimes.empty();
}

CUIPartObject* CUIAnimatorCom::Find_Part(const _wstring& strPartTag) const
{
	auto iter = m_Parts.find(strPartTag);
	if (iter == m_Parts.end())
		return nullptr;

	return iter->second;
}

void CUIAnimatorCom::Restore_BounceBase(const UI_BOUNCE_RUNTIME& Runtime)
{
	if (!Runtime.pTarget || !Runtime.pTarget->Get_Transform())
		return;

	Runtime.pTarget->Get_Transform()->Set_State(
		STATE::POSITION,
		XMVectorSet(Runtime.vBasePos.x, Runtime.vBasePos.y, Runtime.fBaseZ, 1.f));
}

void CUIAnimatorCom::Restore_FadeBase(const UI_FADE_RUNTIME& Runtime)
{
	Try_SetAlpha(Runtime.pTarget, Runtime.fBaseAlpha);
}

_bool CUIAnimatorCom::Try_GetAlpha(CUIPartObject* pPart, _float* pOutAlpha) const
{
	if (!pPart || !pOutAlpha)
		return false;

	for (const Engine::FPROPERTY& Property : pPart->Get_Properties())
	{
		if (Property.strName != L"Alpha" || Property.eType != Engine::PROP_TYPE::FLOAT)
			continue;

		const auto* pAlpha =
			static_cast<const _float*>(pPart->Get_PropertyPtr(Property.uOffset));

		if (!pAlpha)
			return false;

		*pOutAlpha = max(0.f, min(1.f, *pAlpha));
		return true;
	}

	for (const Engine::FPROPERTY& Property : pPart->Get_Properties())
	{
		if (Property.strName != L"Color" || Property.eType != Engine::PROP_TYPE::FLOAT4)
			continue;

		const auto* pColor =
			static_cast<const _float4*>(pPart->Get_PropertyPtr(Property.uOffset));

		if (!pColor)
			return false;

		*pOutAlpha = max(0.f, min(1.f, pColor->w));
		return true;
	}

	return false;
}

_bool CUIAnimatorCom::Try_SetAlpha(CUIPartObject* pPart, _float fAlpha) const
{
	if (!pPart)
		return false;

	fAlpha = max(0.f, min(1.f, fAlpha));

	for (const Engine::FPROPERTY& Property : pPart->Get_Properties())
	{
		if (Property.strName != L"Alpha" || Property.eType != Engine::PROP_TYPE::FLOAT)
			continue;

		auto* pAlpha = static_cast<_float*>(pPart->Get_PropertyPtr(Property.uOffset));
		if (!pAlpha)
			return false;

		*pAlpha = fAlpha;
		return true;
	}

	for (const Engine::FPROPERTY& Property : pPart->Get_Properties())
	{
		if (Property.strName != L"Color" || Property.eType != Engine::PROP_TYPE::FLOAT4)
			continue;

		auto* pColor = static_cast<_float4*>(pPart->Get_PropertyPtr(Property.uOffset));
		if (!pColor)
			return false;

		pColor->w = fAlpha;
		return true;
	}

	return false;
}

CUIAnimatorCom* CUIAnimatorCom::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUIAnimatorCom* pInstance = new CUIAnimatorCom(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUIAnimatorCom");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CUIAnimatorCom::Clone(void* pArg)
{
	CUIAnimatorCom* pInstance = new CUIAnimatorCom(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUIAnimatorCom");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUIAnimatorCom::Free()
{
	__super::Free();
}
