#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CRangerEnemy_Body;

class CRangerEnemy final : public CMonster
{
public:
    struct RANGERENEMY_DESC : public CContainerObject::COTAINEROBJECT_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_RangerEnemy";

private:
    CRangerEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRangerEnemy(const CRangerEnemy& Prototype);
    virtual ~CRangerEnemy() = default;

protected:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;

public:
    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    virtual _float          Get_CapsuleRadius() const override  { return 0.6f; }
    virtual _float          Get_CapsuleHeight() const override  { return 1.0f; }
    virtual _float          Get_InteractRadius() const override { return 5.0f; }

    virtual _bool           Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;
    virtual CAnimator*      Get_BodyAnimator() const override;

protected:
    virtual CMonsterBrain*  Create_Brain() override;
    virtual HRESULT         Ready_State() override;
    virtual HRESULT         Ready_PartObjects() override;

private:
    CRangerEnemy_Body*      m_pBody = { nullptr };

public:
    static CRangerEnemy*    Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END
