#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Client)

class CLD_MeteorGenerator final : public CLevelDesignObject
{
    GENERATED_BODY(CLD_MeteorGenerator)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_MeteorGenerator";
    static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
    CLD_MeteorGenerator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLD_MeteorGenerator(const CLD_MeteorGenerator& Prototype);
    virtual ~CLD_MeteorGenerator() = default;

    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
    static void Register_LevelDesignSpecs();
    static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    static CLD_MeteorGenerator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END
