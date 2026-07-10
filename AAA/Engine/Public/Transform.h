#pragma once

#include "Component.h"

/* 1. 객체의 월드 상태를 표현해주는 상태변환행렬을 보관한다.(월드변환행렬) */
/* 2. 월드행렬의 상태 표현을 위한 여러 인터페이스를 보관한다. */

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Engine)

class CNavigation;

class ENGINE_DLL CTransform final : public CComponent
{
	GENERATED_BODY(CTransform)
	PROPERTY(_float, m_fSpeedPerSec, L"Speed/sec", L"Default")
	PROPERTY(_float, m_fRotationPerSec, L"Rotation/sec/dgree", L"Default")

public:
	typedef struct tagTransformDesc
	{
		_float		fScale = { 1.f };
		_float		fSpeedPerSec = {};
		_float		fRotationPerSec = {};

		_float4     vRight		= { 1.f, 0.f, 0.f, 0.f };
		_float4     vUp			= { 0.f, 1.f, 0.f, 0.f };
		_float4     vLook		= { 0.f, 0.f, 1.f, 0.f };
		_float4     vPosition	= { 0.f, 0.f, 0.f, 1.f };
	}TRANSFORM_DESC;

protected:
	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransform(const CTransform& Prototype);
	virtual ~CTransform() = default;

public:
	_vector Get_State(STATE eState) {
		//return XMLoadFloat4(static_cast<_float4*>(&m_WorldMatrix.m[ETOUI(eState)]));
		return XMLoadFloat4(&m_States[ETOUI(eState)]);
	}

	_float3 Get_Scaled() {
		return _float3(
			XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
			XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
			XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
		);
	}

	const _float4x4* Get_WorldMatrixPtr() const {
		return &m_WorldMatrix;
	}

	_float Get_SpeedPerSec() const { return m_fSpeedPerSec; }

	void Set_State(STATE eState, _fvector vState) {
		//XMStoreFloat4(static_cast<_float4*>(&m_WorldMatrix.m[ETOUI(eState)]), vState);
		XMStoreFloat4(&m_States[ETOUI(eState)], vState);
	}

	void Set_WorldMatrix(const _fmatrix Matrix) {
		XMStoreFloat4x4(&m_WorldMatrix, Matrix);
	}

	void Set_WorldMatrix(const _float4x4& Matrix) {
		m_WorldMatrix = Matrix;
	}

	void Set_RotationPerSec(_float fDegPerSec) { m_fRotationPerSec = fDegPerSec; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName);

public:
	void Set_Scale(_float fScaleX = 1.f, _float fScaleY = 1.f, _float fScaleZ = 1.f);
	void Scaling(_float fScaleX = 1.f, _float fScaleY = 1.f, _float fScaleZ = 1.f);

	void Go_Straight(_float fTimeDelta, CNavigation* pNavigation = nullptr);
	void Go_Backward(_float fTimeDelta, CNavigation* pNavigation = nullptr);
	void Go_Left(_float fTimeDelta);
	void Go_Right(_float fTimeDelta);
	_bool Go_ToPoint(_fvector vTargetPos, _float fTimeDelta, CNavigation* pNavigation = nullptr);
	_bool Follow_Waypoints(deque<_float3>& Waypoints, _float fTimeDelta, CNavigation* pNavi = nullptr);

	void Go_Dir(_float fTimeDelta, _fvector vDir);

	void Rotation(_fvector vAxis, _float fRadian);
	void Rotation(_fvector vQuatanion);
	void Rotate(_fvector vQuatanion);
	void Turn(_fvector vAxis, _float fTimeDelta);
	_matrix Get_RotationMatrix();

	void Chase(_fvector vGoal, _float fTimeDelta, _float fLimit = 0.1f, class CNavigation* pNavigation = nullptr);
	void Chase_XZ(_fvector vGoal, _float fTimeDelta, _float fLimit = 0.1f, class CNavigation* pNavigation = nullptr);

	void LookAt(_fvector vAt);
	_bool LookAt_Smooth(_fvector vAt, _float fTimeDelta);

	void Remove_YRotation();

	void LookTo(_fvector vLookDir, _fvector vUpDir);
	void LookTo(_fvector vLookDir);



private:
	union {
		_float4x4				m_WorldMatrix = {};
		_float4					m_States[ETOUI(STATE::END)];
	};
	
public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END