#pragma once
#include "MultiHitBoxPart.h"

NS_BEGIN(Client)

class CBoss_Armadillo_Body final : public CMultiHitBoxPart
{
    GENERATED_BODY(CBoss_Armadillo_Body)

public:
    enum ARMADILLO_HITBOX { AHB_ROLL, AHB_CATCH, AHB_THROW, AHB_END };

private:
    CBoss_Armadillo_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Armadillo_Body(const CBoss_Armadillo_Body& Prototype);
    virtual ~CBoss_Armadillo_Body() = default;

public:
    struct BOSS_ARMADILLO_BODY_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Armadillo_Body";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Armadillo_Body";
    static constexpr const wchar_t* PART_TAG = L"Body";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    enum ARMADILLO_PASS : _uint { PASS_SHADOW = 0, PASS_BODY = 1, PASS_EYE = 2, PASS_SHELL = 3 };

private:
    HRESULT Ready_Components();
    HRESULT Bind_MeshMaterials(_uint iMeshIndex, _uint iPass);
    HRESULT Bind_PBRSet(_uint iMeshIndex, _uint iSetIndex);

public:
    static CBoss_Armadillo_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Armadillo_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END