#pragma once

#include "GameContent_Defines.h"

#include "Effect_RectParticle.h"

NS_BEGIN(Client)

class CStar2DParticle final : public CEffect_RectParticle
{
	GENERATED_BODY(CStar2DParticle)

PROPERTY(_float, m_fStarArcRadius, L"Arc Radius", L"Star Arc");
PROPERTY(_float, m_fStarStartAngle, L"Start Angle", L"Star Arc");
PROPERTY(_float, m_fStarArcDegrees, L"Arc Degrees", L"Star Arc");
PROPERTY(_float, m_fStarSpawnIntervalRatio, L"Spawn Interval Ratio", L"Star Arc");
PROPERTY(_float, m_fStarLeaderTravelRatio, L"Leader Travel Ratio", L"Star Arc");
PROPERTY(_float, m_fStarLeaderLifeRatio, L"Leader Life Ratio", L"Star Arc");
PROPERTY(_float, m_fStarTrailLifeRatio, L"Trail Life Ratio", L"Star Arc");
PROPERTY(_float, m_fStarTrailDriftDistance, L"Trail Drift Distance", L"Star Arc");
PROPERTY(_float, m_fStarLeaderSizeScale, L"Leader Size Scale", L"Star Arc");
PROPERTY(_float, m_fStarTrailSizeScale, L"Trail Size Scale", L"Star Arc");

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Star2DParticle";

private:
	CStar2DParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStar2DParticle(const CStar2DParticle& Prototype);
	virtual ~CStar2DParticle() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

protected:
	virtual void On_Deserialized() override;
	virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;

public:
	static CStar2DParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	void Init_PropertyValue();

	static constexpr _uint STAR_COUNT = 4;

	virtual void Free();
};

NS_END
