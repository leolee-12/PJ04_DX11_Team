#pragma once

#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CSplit_Starblock final : public CEffect_Container
{
    GENERATED_BODY(CSplit_Starblock)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Split_Starblock";

private:
    CSplit_Starblock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSplit_Starblock(const CSplit_Starblock& Prototype);
    virtual ~CSplit_Starblock() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_EffectPartObjects();

public:
    static CSplit_Starblock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
private:
    virtual void Free() override;
};

NS_END