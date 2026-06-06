#pragma once
#include "GameContent_Defines.h"
#include "UIPartObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CUI_Image final : public CUIPartObject
{
	GENERATED_BODY(CUI_Image)

	PROPERTY(_float4, m_vColor, L"Color", L"UI");
	PROPERTY(_float, m_fAlpha, L"Alpha", L"UI");

	PROPERTY(_int, m_iTextureLevel, L"TextureLevel", L"Texture");
	PROPERTY(_wstring, m_strTextureProtoTag, L"TextureProtoTag", L"Texture");

public:
	typedef struct tagUIImageDesc : public CUIPartObject::UI_PARTOBJECT_DESC
	{
		_uint			iTextureLevel = {};						// 텍스처 프로토 레벨 
		const _tchar*	szTextureProtoTag = { nullptr };		// 텍스처 프로토 태그 
		_float2			vSize = { 100.f, 100.f };
		_float2			vPosition = { 0.f, 0.f };
		_uint			iRenderLayer = { 1 };					// RENDERUIID	 (0 : Back / 1 : MIDDLE / 2 : FRONT)
	}UI_IMAGE_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_Image";

protected:
	CUI_Image(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Image(const CUI_Image& Prototype);
	virtual ~CUI_Image() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDetla) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual void			Deserialize(const json& j) override;
	virtual void			Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	static CUI_Image*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

private:
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom = { nullptr };

private:
	HRESULT					Ready_Components(UI_IMAGE_DESC* pDesc);
	HRESULT					Bind_ShaderResources();

protected:
	virtual void			Free() override;
};

NS_END