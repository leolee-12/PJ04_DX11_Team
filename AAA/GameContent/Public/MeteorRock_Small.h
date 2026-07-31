#pragma once
#include "MeteorRock.h"

class CMeteorRock_Small final : public CMeteorRock
{
    GENERATED_BODY(CMeteorRock_Small)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MeteorRock_Small";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_VolcanoRock_Small";
    static constexpr const _tchar* SND_BREAK_SMALL = L"GimmickVolcanoRock_SmallBreak.wav";

private:
    CMeteorRock_Small(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMeteorRock_Small(const CMeteorRock_Small& Prototype);
    virtual ~CMeteorRock_Small() = default;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }
    
protected:
    virtual const _tchar*       Get_ModelProtoTag() override { return MODEL_PROTO_TAG; }
    virtual const _tchar*       Get_BreakSoundKey() override { return SND_BREAK_SMALL; }

public:
    static CMeteorRock_Small*   Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void Free() override;
};