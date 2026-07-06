#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
	GENERATED_BODY_ABSTRACT(CCamera)
	PROPERTY(_float, m_fFovy, L"FovY", L"CameraOption")
	PROPERTY(_float, m_fNear, L"Near", L"CameraOption")
	PROPERTY(_float, m_fFar, L"Far", L"CameraOption")
	PROPERTY(_float3, m_vEye, L"Eye", L"CameraOption")
	PROPERTY(_float3, m_vAt, L"At", L"CameraOption")

	PROPERTY(_float, m_fShadowRange,    L"ShadowRange",    L"Shadow")
	PROPERTY(_float, m_fShadowPadding,  L"ShadowPadding",  L"Shadow")
	PROPERTY(_uint,  m_iShadowRes,      L"ShadowRes",      L"Shadow")
	PROPERTY(_bool,  m_bDriveShadowFit, L"DriveShadowFit", L"Shadow")

public:
	typedef struct tagCameraDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float3			vEye, vAt;
		_float			fFovy, fNear, fFar;
	}CAMERA_DESC;
protected:
	CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera(const CCamera& Prototype);
	virtual ~CCamera() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Deserialize_Internal(const json& j) override;

public:
	float Get_Fovy() const { return m_fFovy; }

public:
	void Recalculate_ProjMatrix();
	SHADOW_LIGHT_DESC Make_CameraFit_Shadow(const _float4& vLightDir, _float fRange, _float fPadding, _uint iRes) const;

protected:	
	_float4x4		m_ProjMatrix = {};

	_float3 m_vShadowDir = { 0.071f, -0.940f, 0.335f };
	_float  m_fShadowMaxRadius = { 80.f };

protected:
	void Update_PipeLine();
	void Update_ShadowFit();

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END