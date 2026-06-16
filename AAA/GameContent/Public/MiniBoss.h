#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Client)

enum class EMINIBOSS_LIFE { HIDDEN, INTRO, ACTIVE };

class CMiniBoss abstract : public CMonster
{
    GENERATED_BODY_ABSTRACT(CMiniBoss)

protected:
    CMiniBoss(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
    CMiniBoss(const CMiniBoss& Prototype);
    virtual ~CMiniBoss() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;

public:
    void            Appear();                              // 룸 트리거가 호출
    EMINIBOSS_LIFE  Get_Life() const { return m_eLife; }
    _bool           Is_Active() const { return m_eLife == EMINIBOSS_LIFE::ACTIVE; }

protected:
    virtual void    Update_AI(_float fTimeDelta) override; // 라이프사이클 게이팅

    virtual CMonsterBrain* Create_Brain() override = 0;    
    virtual _bool          Use_StateMachine() const override { return false; }

    virtual HRESULT        Ready_Parts() = 0;             
    virtual void           Play_Intro() = 0;              
    virtual _bool          Is_Intro_Finished() const = 0;  
    virtual const _tchar* Get_AppearEventTag() const { return nullptr; } 

protected:
    EMINIBOSS_LIFE m_eLife = { EMINIBOSS_LIFE::HIDDEN };
    _bool          m_bIntroStarted = { false };

protected:
    virtual void Free() override;
};

NS_END