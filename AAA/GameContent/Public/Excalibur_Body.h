#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

// 꽂힌 검 메쉬 파트 (갈락시아 소드 모델 재사용)
class CExcalibur_Body final : public CMonsterPart
{
    GENERATED_BODY(CExcalibur_Body)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Excalibur_Body";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Excalibur_Sword";
    static constexpr const _tchar* PART_TAG = L"Body";

private:
    CExcalibur_Body(ID3D11Device*, ID3D11DeviceContext*);
    CExcalibur_Body(const CExcalibur_Body& Prototype);
    virtual ~CExcalibur_Body() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CExcalibur_Body* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CExcalibur_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END