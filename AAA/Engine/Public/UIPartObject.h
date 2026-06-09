#pragma once

#include "UIObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CUIPartObject abstract : public CUIObject
{
public:
      struct BOUNCE_DESC
      {
          _bool   bPlaying = { false };

          _float2 vBasePos = {};
          _float  fBaseZ = { 1.f };

          _float2 vDirection = { 0.f, 1.f };

          _float  fDistance = { 0.f };
          _float  fDuration = { 0.f };
          _float  fElapsed = { 0.f };
          _float  fWaveCount = { 1.f };
          _float  fDamping = { 0.f };

          void Reset()
          {
              bPlaying = false;
              vBasePos = {};
              fBaseZ = 1.f;
              vDirection = { 0.f, 1.f };
              fDistance = 0.f;
              fDuration = 0.f;
              fElapsed = 0.f;
              fWaveCount = 1.f;
              fDamping = 0.f;
          }
      };

public:
	typedef struct tagUIPartObjectDesc : public CUIObject::UIOBJECT_DESC
	{
		const _float4x4* pParentMatrix;
	}UI_PARTOBJECT_DESC;

protected:
	CUIPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIPartObject(const CUIPartObject& Prototype);
	virtual ~CUIPartObject() = default;

public:
    void                    Play_Bounce(const _float2& vDirection, _float fDistance, _float fDuration, _float fWaveCount = 1.f, _float fDamping = 0.f);
    void                    Stop_Bounce(_bool bRestoreBase = true);
    _bool                   Is_Bouncing() const { return m_Bounce.bPlaying; }

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;

public:
	void					Set_ParentMatrix(const _float4x4* pParentMatrix);
	_bool					Has_ParentMatrix() const { return m_pParentMatrix != nullptr; }

protected:
	const _float4x4*		m_pParentMatrix = {};
	_float4x4				m_CombinedWorldMatrix = {};

protected:
	void					Compute_CombinedWorldMatrix(_fmatrix ChildMatrix);
    void                    Update_Bounce(_float fTimeDelta);

protected:
    BOUNCE_DESC             m_Bounce = {};

public:
	virtual CGameObject*	Clone(void* pArg) = 0;
	virtual void			Free();
};

NS_END