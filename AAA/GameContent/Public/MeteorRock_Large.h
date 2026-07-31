#pragma once
#include "MeteorRock.h"

class CMeteorRock_Large final : public CMeteorRock
{
    GENERATED_BODY(CMeteorRock_Large)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MeteorRock_Large";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_VolcanoRock_Large";
    static constexpr const _tchar* SND_BREAK_LARGE = L"GimmickVolcanoRock_LargeBreak.wav";

private:
    CMeteorRock_Large(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMeteorRock_Large(const CMeteorRock_Large& Prototype);
    virtual ~CMeteorRock_Large() = default;

public:
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual const _tchar*       Get_ModelProtoTag() override { return MODEL_PROTO_TAG; }
    virtual const _tchar*       Get_BreakSoundKey() override { return SND_BREAK_LARGE;  }

public:
    static CMeteorRock_Large*   Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};