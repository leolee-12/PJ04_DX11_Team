#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

// È¹µæ ÇÁ·ÒÇÁÆ® 3D UI ÆÄÆ® (°Ë À§ È¸Àü+º¾ºù)
class CExcalibur_GetIt final : public CMonsterPart
{
    GENERATED_BODY(CExcalibur_GetIt)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Excalibur_GetIt";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_GetIt";   // ½ÇÁ¦ UI ¸ðµ¨ ÅÂ±×·Î ±³Ã¼
    static constexpr const _tchar* PART_TAG = L"GetIt";

    static constexpr _float HOVER_HEIGHT = 1.1f;   // °Ë À§ ±âº» ³ôÀÌ (2.2 ¡æ Àý¹Ý)
    static constexpr _float BOB_AMPL = 0.075f;     // º¾ºù ÁøÆø (0.15 ¡æ Àý¹Ý)
    static constexpr _float BOB_SPEED = 3.f;

private:
    CExcalibur_GetIt(ID3D11Device*, ID3D11DeviceContext*);
    CExcalibur_GetIt(const CExcalibur_GetIt& Prototype);
    virtual ~CExcalibur_GetIt() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

private:
    _float m_fAccTime = { 0.f };

public:
    static CExcalibur_GetIt* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CExcalibur_GetIt* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END