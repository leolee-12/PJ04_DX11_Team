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

	virtual void Deserialize(const json& j) override;

public:
	void Recalculate_ProjMatrix();

protected:	
	_float4x4		m_ProjMatrix = {};

	class CPipeLine* m_pPipeLine = { nullptr };

protected:
	void Update_PipeLine();

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END