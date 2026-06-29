#pragma once
#include "Monster_Movement.h"
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

class CMonster_RailMovement final : public CMonster_Movement
{
	GENERATED_BODY(CMonster_RailMovement)

protected:
    CMonster_RailMovement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMonster_RailMovement(const CMonster_RailMovement& Prototype);
    virtual ~CMonster_RailMovement() = default;

public:
    virtual HRESULT                 Initialize_Prototype() override;
    virtual HRESULT                 Initialize(void* pArg) override; 

public:
    void                            Set_Rail(const LD_RAIL_DESC& Desc);
    void                            Clear_Rail();
    _bool                           Has_Rail() const { return m_bHasRail; }
    void                            Set_PathSpeed(_float fSpeed) { m_fPathSpeed = fSpeed; }
    void                            Set_SpinSpeed(_float fDeg) { m_fSpinSpeedDeg = fDeg; }
    void                            Set_FaceTangent(_bool b) { m_bFaceTangent = b; }

    _bool                           Update_RailFollow(_float fTimeDelta);
    _bool                           Is_OffPath(_float fThreshold = 0.5f) const;
    void                            Warp_ToRandomPoint();
    _bool                           Return_TowardPath(_float fTimeDelta, _float fSpeed);
    void                            Snap_ToPath();

protected:
    virtual void                    Calc_Vertical(_float fTimeDelta) override;

private:
    _float                          Compute_PathLength() const;
    _bool                           Eval_PathPos(_float fS, _float3* pOut, _float3* pOutTangent = nullptr) const;
    void                            Face_PathDir(const _float3& vPos, const _float3& vTangent, _float fTimeDelta);

private:
    LD_RAIL_DESC                    m_tRailDesc = {};
    _bool                           m_bHasRail = { false };

    _float                          m_fPathLength = { 0.f };
    _float                          m_fPathS = { 0.f };
    _float                          m_fPathSpeed = { 6.0f };            // 경로 이동 속도
    _int                            m_iPathDir = { 1 };

    _float                          m_fSpinSpeedDeg = { 0.f };
    _float                          m_fSpinAngle = { 0.f };
    _bool                           m_bFaceTangent = { false };

public:
    static CMonster_RailMovement*   Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent*             Clone(void* pArg) override;

protected:
    virtual void                    Free() override;
};

NS_END