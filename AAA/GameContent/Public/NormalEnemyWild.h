#pragma once
#include "NormalEnemy.h"

NS_BEGIN(Client)

class CNormalEnemyWild final : public CNormalEnemy
{
	GENERATED_BODY(CNormalEnemyWild)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_NormalEnemyWild";

private:
    CNormalEnemyWild(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CNormalEnemyWild(const CNormalEnemyWild& Prototype);
    virtual ~CNormalEnemyWild() = default;

protected:
    virtual HRESULT             Initialize(void* pArg) override;

    virtual HRESULT             Ready_PartObjects() override;

public:
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    static CNormalEnemyWild*    Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END