#pragma once
#include "BossNamePlate.h"

NS_BEGIN(Client)

class CMetaknightNamePlate final : public CBossNamePlate
{
    GENERATED_BODY(CMetaknightNamePlate)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_MetaknightNamePlate";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_NameLocator";

    static constexpr _float3 BONE_OFFSET = { 0.f, 1.17604f, -8.09389f };
    static constexpr _float  BONE_SCALE = 0.2f;

private:
    CMetaknightNamePlate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMetaknightNamePlate(const CMetaknightNamePlate& Prototype);
    virtual ~CMetaknightNamePlate() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Events() override;
    virtual const _tchar* Get_ModelProtoTag() const override
    {
        return MODEL_PROTO_TAG;
    }
    virtual const _tchar* Get_ActivateEventTag() const override
    {
        return TEXT("Metaknight.NamePlateOn");
    }

private:
    const _float4x4* m_pAnchor = { nullptr };

    static constexpr _float4 s_vColorBlack = { 0.f, 0.f, 0.f, 1.f };
    //static constexpr _float4 s_vColorBlue = { 93.f / 255.f, 101.f / 255.f, 1.f, 1.f };
    static constexpr _float4 s_vColorBlue = { 12.f / 255.f, 16.f / 255.f, 135.f / 255.f, 1.f };
    static constexpr _float4 s_vColorWhite = { 1.f, 1.f, 1.f, 1.f };

public:
    static CMetaknightNamePlate* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END