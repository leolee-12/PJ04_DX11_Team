#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CCustomFont;

class CFont_Manager final : public CBase
{
private:
	CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFont_Manager() = default;

public:
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT Draw(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition,
		_fvector vColor, _float fRotation, const _float2& vScale, TEXT_ALIGN eAlign = TEXT_ALIGN::CENTER);
	HRESULT Draw_Raw(const _wstring& tag, const _tchar* p, const _float2& pos, _fvector col, const _float2& scl,
		TEXT_ALIGN a);

	_float2 Measure(const _wstring& strFontTag, const _tchar* pText);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

private:
	unordered_map<_wstring, CCustomFont*>		m_Fonts;

private:
	CCustomFont* Find_Font(const _wstring& strFontTag);

public:
	static CFont_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END