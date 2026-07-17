#pragma once
#include "NormalEnemy_Body.h"

NS_BEGIN(Client)

class CNormalEnemyWild_Body final : public CNormalEnemy_Body
{
    GENERATED_BODY(CNormalEnemyWild_Body)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_NormalEnemyWild_Body";

private:
    CNormalEnemyWild_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CNormalEnemyWild_Body(const CNormalEnemyWild_Body& Prototype);
    virtual ~CNormalEnemyWild_Body() = default;

protected:
    virtual HRESULT Ready_Components() override;

public:
    virtual void                        Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    static CNormalEnemyWild_Body*       Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*                Clone(void* pArg) override;

protected:
    virtual void                        Free() override;
};

NS_END