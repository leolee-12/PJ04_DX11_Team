#include "Effect.h"
#include "GameInstance.h"

CEffect::CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
}

CEffect::CEffect(const CEffect& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CEffect::Initialize(void* pArg)
{
    // 베이스가 Transform 컴포넌트 생성 + TRANSFORM_DESC 부분 초기화
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (nullptr == pArg)
        return E_FAIL;

    EFFECT_DESC* pDesc = static_cast<EFFECT_DESC*>(pArg);

    // 위치
    _vector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);

    // forward → Look 축. up 레퍼런스로 (0,1,0) 사용
    _vector vLook = XMVector3Normalize(XMLoadFloat3(&pDesc->vForward));
    _vector vUpRef = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    // forward 가 거의 수직(±Y) 이면 cross 가 0 이 되어 회전 행렬이 망가짐 → 보조 축으로 폴백
    _vector vRight = XMVector3Cross(vUpRef, vLook);
    if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-6f)
        vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
    vRight = XMVector3Normalize(vRight);

    _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    const _float3& s = pDesc->vScale;
    m_pTransformCom->Set_State(STATE::RIGHT, vRight * s.x);
    m_pTransformCom->Set_State(STATE::UP, vUp * s.y);
    m_pTransformCom->Set_State(STATE::LOOK, vLook * s.z);

    // 라이프타임/방향 캐시
    m_fLifeTime = pDesc->fLifeTime;
    m_vForward = pDesc->vForward;

    // 부착 정보
    m_pAttachBone = pDesc->pAttachBone;
    m_pAttachOwner = pDesc->pAttachOwner;
    m_vAttachOffset = pDesc->vAttachOffset;
    if (m_pAttachOwner)
        Safe_AddRef(m_pAttachOwner);

    On_Spawn();
    return S_OK;
}

void CEffect::Update(_float fTimeDelta)
{
    m_fAge += fTimeDelta;

    On_Tick(fTimeDelta);   // 구체 클래스가 여기서 Set_Dead 호출할 수도 있음

    if (m_fLifeTime > 0.f && m_fAge >= m_fLifeTime)
        Set_Dead();

    if (Is_Dead() && !m_bExpireFired)
    {
        m_bExpireFired = true;
        On_Expire();       // 정확히 한 번 보장 (Tick 안 Set_Dead 든 라이프타임 만료든)
    }
}

void CEffect::Late_Update(_float fTimeDelta)
{
    // 본 연속 부착이 설정된 경우에만 매 프레임 동기화
    if (m_pAttachBone && m_pAttachOwner)
    {
        _matrix mBone = XMLoadFloat4x4(m_pAttachBone);
        _matrix mOwner = XMLoadFloat4x4(m_pAttachOwner->Get_Transform()->Get_WorldMatrixPtr());
        _matrix mWorld = mBone * mOwner;

        // 본 로컬 오프셋을 본 회전축으로 변환해 위치에 누적
        _vector vOffset = XMVector3TransformNormal(XMLoadFloat3(&m_vAttachOffset), mWorld);
        mWorld.r[3] = XMVectorAdd(mWorld.r[3], vOffset);

        m_pTransformCom->Set_WorldMatrix(mWorld);
    }

    // Add_RenderGroup 은 구체 클래스가 자기 셰이더 종류에 맞춰 호출 (BLEND / NONBLEND 등)
    // 베이스에서 강제하지 않음.
}

void CEffect::Free()
{
    Safe_Release(m_pAttachOwner);
    __super::Free();
}