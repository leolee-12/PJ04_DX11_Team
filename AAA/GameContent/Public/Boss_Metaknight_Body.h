#pragma once
#include "MultiHitBoxPart.h"

NS_BEGIN(Client)

class CBoss_Metaknight_Body final : public CMultiHitBoxPart
{
    GENERATED_BODY(CBoss_Metaknight_Body)

public:
    enum METAKNIGHT_HITBOX { MKHB_SLASH, MKHB_CATCH, MKHB_END };

private:
    CBoss_Metaknight_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Metaknight_Body(const CBoss_Metaknight_Body& Prototype);
    virtual ~CBoss_Metaknight_Body() = default;

public:
    struct BOSS_METAKNIGHT_BODY_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight_Body";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Metaknight_Body";
    static constexpr const wchar_t* PART_TAG = L"Body";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    enum METAKNIGHT_PASS : _uint { PASS_SHADOW = 0, PASS_BODY = 1, PASS_WING = 2, PASS_SHOULDER = 3, PASS_EYE = 4, PASS_FACE = 5 };

private:
    HRESULT Ready_Components();
    HRESULT Bind_MeshMaterials(_uint iMeshIndex, _uint iPass);
    HRESULT Bind_PBRSet(_uint iMeshIndex);

public:
    static CBoss_Metaknight_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Metaknight_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END