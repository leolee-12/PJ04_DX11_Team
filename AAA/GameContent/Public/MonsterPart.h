#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"
#include "GameContent_const.h"   // SHADER_DESC

NS_BEGIN(Engine)
class CShader; class CModel; class CAnimator;
NS_END

NS_BEGIN(Client)

// 몬스터/보스 메쉬 파츠 공통 베이스
class CMonsterPart abstract : public CPartObject
{
    GENERATED_BODY_ABSTRACT(CMonsterPart)

public:
    struct MONSTERPART_DESC : public CPartObject::PARTOBJECT_DESC
    {
        const _float4x4* pSocketBoneMatrix = { nullptr }; // 소켓 부착 파츠만 사용(없으면 부모행렬)
        const _float* pHitFlash = { nullptr }; 
        const _float3* pHitFlashColor = { nullptr };
    };

protected:
    struct PART_SETUP
    {
        SHADER_DESC   tShader;                       // Shader_AnimMesh_PBR / Shader_NonAnimMesh_PBR
        const _tchar* szModelProtoTag = nullptr;
        _bool         bAnimated = true;              // 애니 파츠 여부의 단일 기준(정적이면 false)
        const _tchar* szAnimEventFile = nullptr;     // 옵션: 이벤트 트랙 json 경로(생성 여부엔 무관)
    };

protected:
    CMonsterPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMonsterPart(const CMonsterPart& Prototype);
    virtual ~CMonsterPart() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;   // 소켓 행렬 수령 + super
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;                  // 특수 렌더 필요 시 자식이 오버라이드
    virtual HRESULT Render_Shadow() override;

public:
    CAnimator*          Get_Animator() const { return m_pAnimatorCom; }
    CModel*             Get_Model()    const { return m_pModelCom; }
    const _float4x4*    Get_BoneMatrixPtr(const _char* pBoneName) const;

protected:
    HRESULT         Ready_MeshPart(const PART_SETUP& tSetup);
    virtual HRESULT Ready_Components() = 0;                     // 모든 구현 클래스에서 본인 Ready_Components 함수 구현 필수
    virtual HRESULT Bind_ShaderResources();

protected:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };
    const _float4x4* m_pSocketBoneMatrix = { nullptr };

    const _float* m_pHitFlash = { nullptr };
    const _float3* m_pHitFlashColor = { nullptr };

protected:
    virtual void Free() override;
};

NS_END