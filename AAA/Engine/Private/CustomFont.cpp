#include "CustomFont.h"
#include "GameInstance.h"

CCustomFont::CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCustomFont::Initialize(const _tchar* pFontFilePath)
{
	m_pBatch = new SpriteBatch(m_pContext);
	m_pFont = new SpriteFont(m_pDevice, pFontFilePath);

	const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::ORTHO);
	m_fDesignW = 2.f / pProj->_11;
	m_fDesignH = 2.f / pProj->_22;

	return S_OK;
}

HRESULT CCustomFont::Draw(const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vScale, TEXT_ALIGN eAlign)
{
	D3D11_VIEWPORT vp;
	UINT numVP = 1;
	m_pContext->RSGetViewports(&numVP, &vp);
	_float fActualW = vp.Width;
	_float fActualH = vp.Height;

	_float2 vScreenPos;
	vScreenPos.x = (vPosition.x + m_fDesignW * 0.5f) / m_fDesignW * fActualW;
	vScreenPos.y = (m_fDesignH * 0.5f - vPosition.y) / m_fDesignH * fActualH;

	XMVECTOR size = m_pFont->MeasureString(pText);
	_float fW = XMVectorGetX(size);
	_float fH = XMVectorGetY(size);

	_float fOriginX = fW * 0.5f;	// 기본 배치는 CENTER

	if (eAlign == TEXT_ALIGN::LEFT)
		fOriginX = 0.f;
	else if (eAlign == TEXT_ALIGN::RIGHT)
		fOriginX = fW;

	_float2 vOrigin = { fOriginX, fH * 0.5f };	// 세로는 중앙 유지

	m_pBatch->Begin();

	m_pFont->DrawString(m_pBatch, pText, vScreenPos, vColor, fRotation, vOrigin, vScale);

	m_pBatch->End();

	return S_OK;
}

_float2 CCustomFont::Measure(const _tchar* pText) const
{
	if (nullptr == m_pFont || nullptr == pText)
		return _float2(0.f, 0.f);

	XMVECTOR v = m_pFont->MeasureString(pText);
	return _float2(XMVectorGetX(v), XMVectorGetY(v));
}

CCustomFont* CCustomFont::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFontFilePath)
{
	CCustomFont* pInstance = new CCustomFont(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pFontFilePath)))
	{
		MSG_BOX("Failed to Created : CCustomFont");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CCustomFont::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance_Proxy);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Delete(m_pBatch);
	Safe_Delete(m_pFont);
}
