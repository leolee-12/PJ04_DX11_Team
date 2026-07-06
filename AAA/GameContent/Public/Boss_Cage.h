#pragma once
#include "GameContent_Defines.h"
#include "ContainerObject.h"

NS_BEGIN(Client)

// 골드 케이지 컨테이너: 트랜스폼/부착 로직 소유. 메쉬는 CBoss_Cage_Body 파트가 담당
class CBoss_Cage final : public CContainerObject
{
    GENERATED_BODY(CBoss_Cage)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Boss_Cage";

private:
    CBoss_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Cage(const CBoss_Cage& Prototype);
    virtual ~CBoss_Cage() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;

    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    // 외부 본(고릴라 등)에 부착. OffsetMatrix는 본 기준 로컬 오프셋
    void  Attach_To_Bone(const _float4x4* pBoneMatrix, const _float4x4* pOwnerWorld, _fmatrix OffsetMatrix = XMMatrixIdentity());
    void  Detach();     // 분리 후엔 마지막 합성 월드가 트랜스폼에 남음 (낙하 연출 등에 사용)
    _bool Is_Attached() const { return m_pAttachBone != nullptr; }

private:
    HRESULT Ready_PartObjects();

private:
    const _float4x4* m_pAttachBone = { nullptr };
    const _float4x4* m_pAttachOwnerWorld = { nullptr };
    _float4x4        m_AttachOffset = {};

public:
    static CBoss_Cage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END