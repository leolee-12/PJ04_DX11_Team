#pragma once
#include "GameContent_Defines.h"
#include "UIPartObject.h"
#include "ICurtainPart.h"

NS_BEGIN(Engine)
class CShader; class CTexture; class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

// 커튼 RT용 회전+축소 애니 공통 베이스.
// Active 유지하며 자기 StartDelay 카운트(대기 중 렌더만 스킵), 끝나면 Is_Finished.
// 렌더 패스/머티리얼 바인딩만 서브클래스가 결정.
class CLIENT_DLL CUI_CurtainAnimBase abstract : public CUIPartObject, public ICurtainPart
{
      GENERATED_BODY_ABSTRACT(CUI_CurtainAnimBase)

      PROPERTY(_float, m_fAlpha, L"Alpha", L"UI");
      PROPERTY(_int, m_iTextureLevel, L"TextureLevel", L"Texture");
      PROPERTY(_wstring, m_strTextureProtoTag, L"TextureProtoTag", L"Texture");
      PROPERTY(_bool, m_bPlay, L"Play", L"Anim");
      PROPERTY(_bool, m_bLoop, L"Loop", L"Anim");

      PROPERTY(_float, m_fStartSize, L"StartSize", L"Anim");
      PROPERTY(_float, m_fEndSize, L"EndSize", L"Anim");
      PROPERTY(_float, m_fShrinkDuration, L"ShrinkDuration", L"Anim");
      PROPERTY(_bool, m_bUseAlphaFade, L"UseAlphaFade", L"Anim"); // 쉬링크에 맞춰 알파 보간(off면 기존 동작)
      PROPERTY(_float, m_fStartAlpha, L"StartAlpha", L"Anim"); // t=0 일 때 알파
      PROPERTY(_float, m_fEndAlpha, L"EndAlpha", L"Anim"); // t=1 일 때 알파
      PROPERTY(_bool, m_bAlphaInOut, L"AlphaInOut", L"Anim"); // on: Start→End→Start (sin 험프)
      
      PROPERTY(_float, m_fSpinSpeedDeg, L"SpinSpeedDeg", L"Anim");
      PROPERTY(_bool, m_bSpinEase, L"SpinEase", L"Anim"); // 사인 가속 사용 여부
      PROPERTY(_float, m_fSpinEaseCycle, L"SpinEaseCycle", L"Anim"); // 느림→빠름→느림 한 주기(초)
      PROPERTY(_float, m_fSpinEaseMin, L"SpinEaseMin", L"Anim"); // 최저 속도 배율(0~1)

      PROPERTY(_float, m_fStartDelay, L"StartDelay", L"Anim");
      PROPERTY(_bool, m_bDisableOnFinish, L"DisableOnFinish", L"Anim");
      PROPERTY(_bool, m_bShowWhileWaiting, L"ShowWhileWaiting", L"Anim");

public:
      typedef struct tagUICurtainAnimDesc : public CUIPartObject::UI_PARTOBJECT_DESC
      {
              _uint                   iTextureLevel = {};
              const _tchar* szTextureProtoTag = { nullptr };
              _float                  fAlpha = { 1.f };
              _float2                 vPosition = { 0.f, 0.f };
              _float                  fStartSize = { 600.f };
              _float                  fEndSize = { 0.f };
              _float                  fShrinkDuration = { 1.2f };
              _bool                   bUseAlphaFade = { false };
              _float                  fStartAlpha = { 1.f };
              _float                  fEndAlpha = { 0.f };
              _bool                   bAlphaInOut = { false };

              _float                  fSpinSpeedDeg = { 120.f };
              _bool                   bSpinEase = { false };
              _float                  fSpinEaseCycle = { 1.f };
              _float                  fSpinEaseMin = { 0.f };

              _float                  fStartDelay = { 0.f };
              _bool                   bDisableOnFinish = { true };
              _bool                   bPlay = { false };
              _bool                   bLoop = { false };
              _float                  fZOrder = { 0.5f };
              _uint                   iRenderLayer = { 3 };   // CURTAIN
              _bool                   bShowWhileWaiting = { false };
      }UI_CURTAINANIM_DESC;

protected:
      CUI_CurtainAnimBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
      CUI_CurtainAnimBase(const CUI_CurtainAnimBase& Prototype);
      virtual ~CUI_CurtainAnimBase() = default;

public:
      virtual HRESULT Initialize_Prototype() override;
      virtual HRESULT Initialize(void* pArg) override;
      virtual void    Priority_Update(_float) override {}
      virtual void    Update(_float fTimeDelta) override;
      virtual void    Late_Update(_float fTimeDelta) override;
      virtual HRESULT Render() override;

public:
      void    Set_Loop(_bool b) { m_bLoop = b; }
      void    Anim_Play() { m_bPlay = true; m_bPrevPlay = false; m_bFinished = false; m_bArmed = false; }
      

      HRESULT Set_Texture(_int iLevel, const _wstring& strProtoTag);
      
      virtual _bool   Is_Finished() const override { return m_bFinished; }
      virtual _bool   Is_Loop()     const override { return m_bLoop; }
      virtual void    Anim_Stop() override { m_bPlay = false; }
      virtual void    Begin_Delayed() override { m_bArmed = true; m_fDelayAcc = 0.f; m_bPlay = false; m_bPrevPlay = false; m_bFinished = false; }
      virtual void    Reset() override 
      { 
          m_fAccTime = 0.f;
          m_fSpinAngle = 0.f;
          m_fSpinPhase = 0.f;
          if (m_bUseAlphaFade) 
              m_fAlpha = m_fStartAlpha;
          Reset_Tranform();
      }

protected:
    // ▼ 서브클래스 결정 지점
    virtual _uint   Render_Pass() const = 0;
    virtual HRESULT Bind_Material(CShader* pShader) = 0;

protected:
      CShader* m_pShaderCom = { nullptr };
      CTexture* m_pTextureCom = { nullptr };
      CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

      _float                  m_fAccTime = { 0.f };
      _float                  m_fSpinAngle = { 0.f };
      _float                  m_fSpinPhase = { 0.f };
      _bool                   m_bFinished = { false };
      _bool                   m_bPrevPlay = { false };
      _bool                   m_bPrevLoop = { false };

      _float                  m_fDelayAcc = { 0.f };
      _bool                   m_bArmed = { false };

protected:
      HRESULT                 Ready_Components();
      HRESULT                 Bind_CommonMatrices();
      void                    Reset_Tranform();

protected:
      virtual void    Deserialize_Internal(const json& j) override;
      virtual void    Free() override;
};
NS_END