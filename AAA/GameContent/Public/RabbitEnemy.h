#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CRabbitEnemy_Body;

class CRabbitEnemy final : public CMonster
{
public:
    struct RABBITENEMY_DESC : public CContainerObject::COTAINEROBJECT_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_RabbitEnemy";

private:
    CRabbitEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRabbitEnemy(const CRabbitEnemy& Prototype);
    virtual ~CRabbitEnemy() = default;

protected:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;

public:
    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    virtual _float          Get_CapsuleRadius() const override  { return 1.25f; }
    virtual _float          Get_CapsuleHeight() const override  { return 0.1f; }
    virtual _float          Get_InteractRadius() const override  { return 10.f; }

    virtual _bool           Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual CAnimator*      Get_BodyAnimator() const override;

protected:
    virtual CMonsterBrain*  Create_Brain() override;
    virtual HRESULT         Ready_State() override;
    virtual void            Apply_AIVariation(const _wstring& strVariation) override;

    virtual HRESULT         Ready_PartObjects() override;
    virtual HRESULT         Ready_AnimEvents() override;
    virtual void            On_Exit(MONSTER_STATE_TYPE eNextState) override;

private:
    CRabbitEnemy_Body*      m_pBody = { nullptr };

public:
    static CRabbitEnemy*    Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END
