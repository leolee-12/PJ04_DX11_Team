#pragma once

#include "GameContent_Defines.h"
#include "UIPartObject.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_Text final : public CUIPartObject
{
	GENERATED_BODY(CUI_Text)

	PROPERTY(_float4, m_vColor, L"Color", L"UI");
	PROPERTY(_wstring, m_strText, L"Text", L"Text");
	PROPERTY(_wstring, m_strFontTag, L"FontTag", L"Text");
	PROPERTY(_float, m_fFontScale, L"FontScale", L"Text");
	PROPERTY(_int, m_iAlign, L"Align", L"Text");
	PROPERTY(_float, m_fRotation, L"Rotation", L"Text");

public:
	typedef struct tagUITextDesc : public CUIPartObject::UI_PARTOBJECT_DESC
	{
		const _tchar*				szText = { L"" };
		const _tchar*				szFontTag = { nullptr };
		_float4						vColor = { 1.f, 1.f, 1.f, 1.f };
		_float						fFontScale = { 1.f };
		_int						iAlign = { 1 };				// 0 : L / 1 : C / 2 : R
		_float						fRotation = { 0.f };
		_float2						vPosition = { 0.f, 0.f };
		_uint						iRenderLayer = { 2 };		// FRONT
		_float2						vSize = { 300.f, 100.f };
	}UI_TEXT_DESC;

	static constexpr const _tchar*	PROTOTYPE_TAG = L"Proto_UI_Text";

protected:
	CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Text(const CUI_Text& Prototype);
	virtual ~CUI_Text() = default;

public:
	virtual HRESULT                 Initialize_Prototype() override;
	virtual HRESULT                 Initialize(void* pArg) override;
	virtual void                    Priority_Update(_float fTimeDelta) override;
	virtual void                    Update(_float fTimeDelta) override;
	virtual void                    Late_Update(_float fTimeDelta) override;
	virtual HRESULT                 Render() override;
	virtual void                    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	// 실제 텍스트 크기(MeasureString x FontScale)  -> 에디터 선택 박스용 
	_float2							Get_TextSize() const;
	_float							Get_FontScale() const { return m_fFontScale; }
	void							Set_FontScale(_float fScale)
	{
		m_fFontScale = (fScale > 0.01f) ? fScale : 0.01f;
	}

	_int							Get_Align() const { return m_iAlign; }
	void							Set_Align(_int i) { m_iAlign = i; m_bTextDirty = true; }
	const _wstring&					Get_FontTag() const { return m_strFontTag; }
	void							Set_FontTag(const _wstring& strTag) { m_strFontTag = strTag; m_bTextDirty = true; }

	void							Set_Text(const _wstring& strText) { m_strText = strText; m_bTextDirty = true; }

	void							Mark_TextDirty() { m_bTextDirty = true; }

private:
	CShader* m_pShaderCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	ID3D11Texture2D* m_pTextTex = { nullptr };
	ID3D11RenderTargetView* m_pTextRTV = { nullptr };
	ID3D11ShaderResourceView* m_pTextSRV = { nullptr };
	_uint  m_iTexW = { 0 };
	_uint  m_iTexH = { 0 };
	_bool  m_bTextDirty = { true };

private:
	HRESULT Ready_Components();
	HRESULT Ensure_RT(_uint iW, _uint iH);
	void    Bake_Text();

public:
	static CUI_Text*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;

protected:
	virtual void					Free() override;
};

NS_END