#pragma once
#include "GameContent_Defines.h"
#include "UIPartObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CUI_SpriteAnim final : public CUIPartObject
{
	GENERATED_BODY(CUI_SpriteAnim)

	PROPERTY(_float4, m_vColor, L"Color", L"UI");
	PROPERTY(_float, m_fAlpha, L"Alpha", L"UI");

	PROPERTY(_int, m_iTextureLevel, L"TextureLevel", L"Texture");
	PROPERTY(_wstring, m_strTextureProtoTag, L"TextureProtoTag", L"Texture");

	PROPERTY(_float, m_fDuration, L"Duration", L"Anim");
	PROPERTY(_float, m_fEndDelay, L"EndDelay", L"Anim");
	PROPERTY(_bool, m_bLoop, L"Loop", L"Anim");
	PROPERTY(_bool, m_bAutoPlay, L"AutoPlay", L"Anim");

public:
	typedef struct tagUISpriteAnimDesc : public CUIPartObject::UI_PARTOBJECT_DESC
	{
		_uint           iTextureLevel = {};
		const _tchar*	szTextureProtoTag = { nullptr };
		_float2         vSize = { 100.f, 100.f };
		_float2         vPosition = { 0.f, 0.f };
		_uint           iRenderLayer = { 1 };
		_float          fDuration = { 1.f };
		_float          fEndDelay = { 0.f };
		_bool           bLoop = { true };
		_bool           bAutoPlay = { true };
	}UI_SPRITEANIM_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_SpriteAnim";

protected:
	CUI_SpriteAnim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_SpriteAnim(const CUI_SpriteAnim& Prototype);
	virtual ~CUI_SpriteAnim() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual void			Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

public: // UIContainerObject 제어 API
	void					Play(_bool bLoop);
	void					Pause() { m_bIsPlay = false; }
	void					Resume() { m_bIsPlay = true; m_bFinished = false; }
	void					Stop();
	void					Set_Frame(_int iFrame);
	void					Set_Duration(_float fSec) { m_fDuration = (fSec > 0.f) ? fSec : 0.0001f; }
	void					Set_EndDelay(_float fSec) { m_fEndDelay = (fSec >= 0.f) ? fSec : 0.f; }
	_int					Get_Frame()      const { return m_iTexIndex; }
	_int					Get_FrameCount() const { return m_iFrameCount; }
	_bool					Is_Playing()     const { return m_bIsPlay; }
	_bool					Is_Finished()    const { return m_bFinished; }

	// 에디터에서 데이터 제작 용도
	HRESULT					Set_Texture(_int iLevel, const _wstring& strProtoTag);
	void					Play() { Play(m_bLoop); }		// 현재 Loop 설정으로 재생
	void					Seek(_float fRatio);			// 0 ~ 1 위치로 Seek
	_float					Get_Progress() const;			// 현재 진행도 0 ~ 1

private:
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom = { nullptr };

	_int					m_iFrameCount = { 1 };

	_float					m_fAccTime = { 0.f };
	_bool					m_bIsPlay = { false };
	_bool					m_bFinished = { false };

private:
	HRESULT					Ready_Components(UI_SPRITEANIM_DESC* pDesc);
	HRESULT					Bind_ShaderResources();
	void					Sync_FrameCount();

	virtual void			Deserialize_Internal(const json& j) override;

public:
	static CUI_SpriteAnim*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);

protected:
	virtual void			Free() override;

};

NS_END