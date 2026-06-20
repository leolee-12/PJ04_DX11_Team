#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CBladeKnight_Sword final : public CMonsterPart
{
    GENERATED_BODY(CBladeKnight_Sword)

public:
    struct BLADEKNIGHT_SWORD_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BladeKnight_Sword";
    static constexpr const _tchar* PART_TAG = L"Sword";

private:
    CBladeKnight_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBladeKnight_Sword(const CBladeKnight_Sword& Prototype);
    virtual ~CBladeKnight_Sword() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;                 // 메쉬 1번 특수 패스 때문에 유지
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    HRESULT Ready_Components();

private:
    _float4 m_vConstantDiffuse = { 1.f, 0.72f, 0.08f, 1.f };
    _float3 m_vConstantMRA = { 0.25f, 0.18f, 1.f };
    _float4 m_vConstantEmissive = { 0.05f, 0.025f, 0.f, 1.f };

public:
    static CBladeKnight_Sword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END