#pragma once
#include "GameContent_Defines.h"
#include "BossBase.h"

NS_BEGIN(Client)

class CMonster_State;

class CMiniBoss abstract : public CBossBase
{
    GENERATED_BODY_ABSTRACT(CMiniBoss)

protected:
    CMiniBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMiniBoss(const CMiniBoss& Prototype);
    virtual ~CMiniBoss() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Be_Captured(CGameObject* pInhaler) override;

protected:
    virtual void Update_AI(_float fTimeDelta) override;
    virtual void On_Enter_Corpse() override;            

protected:
    _bool           m_bEaten = { false };         
    CMonster_State* m_pCaptureState = { nullptr };

protected:
    virtual void Free() override;
};

NS_END