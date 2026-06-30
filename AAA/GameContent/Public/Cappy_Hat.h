#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CCappy_Hat final : public CMonsterPart
{
    GENERATED_BODY(CCappy_Hat)

public:
    struct CAPPY_HAT_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Cappy_Hat";

private:
    CCappy_Hat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCappy_Hat(const CCappy_Hat& Prototype);
    virtual ~CCappy_Hat() = default;

private:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;

public:
    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    virtual HRESULT         Ready_Components() override;

public:
    static CCappy_Hat*      Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;

};

NS_END