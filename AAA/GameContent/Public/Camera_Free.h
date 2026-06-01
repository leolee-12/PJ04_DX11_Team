#pragma once

#include "GameContent_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

class CLIENT_DLL CCamera_Free final : public CCamera
{
	GENERATED_BODY(CCamera_Free)

public:
	typedef struct tagCameraFreeDesc final : public CCamera::CAMERA_DESC
	{
		_float		fMouseSensor;
	}CAMERA_FREE_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraFree";

private:
	CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Free(const CCamera_Free& Prototype);
	virtual ~CCamera_Free() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	_float	m_fMouseSensor = {};

private:
	virtual HRESULT Ready_Events() override;

public:
	static CCamera_Free* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free();

};

NS_END