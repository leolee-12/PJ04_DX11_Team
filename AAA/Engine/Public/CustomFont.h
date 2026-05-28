#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;

class CCustomFont final : public CBase
{
private:
	CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCustomFont() = default;

public:
	HRESULT Initialize(const _tchar* pFontFilePath);
	HRESULT Draw(const _tchar* pText, const _float2& vPosition,
		_fvector vColor, _float fRotation, const _float2& vScale);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	SpriteBatch*		m_pBatch = { nullptr };
	SpriteFont*			m_pFont = { nullptr };

	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };
	_float m_fDesignW = {};
	_float m_fDesignH = {};

public:
	static CCustomFont* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFontFilePath);
	virtual void Free() override;

	//MakeSpriteFont "Noto Sans JP ExtraBold" / FontSize:20 / FastPack / CharacterRegion : 0x0020 - 0x00FF / CharacterRegion : 0x3131 - 0x3163 / CharacterRegion : 0xAC00 - 0xD800 / DefaultCharacter : 0xAC00 ER_Default.spritefont
};

NS_END