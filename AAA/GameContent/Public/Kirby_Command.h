#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

NS_BEGIN(Client)

enum class KIRBY_COMMAND_TYPE
{
	NONE,
	MOVE_TOP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
	JUMP,  // A
	ATTACK // B

};

class CLIENT_DLL CKirby_Command abstract : public CBase
{
public:
	virtual ~CKirby_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() = 0;

protected:	
	virtual void Free() { __super::Free(); }
};

class CLIENT_DLL Move_Command abstract : public CKirby_Command
{
public:
	Move_Command(const _float3& vDir) :m_vDir(vDir) {};
	virtual ~Move_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() = 0;

public:
	const _float3& Get_Dir() { return m_vDir; }

protected:
	_float3 m_vDir{};

protected:
	virtual void Free() { __super::Free(); }
};

class CLIENT_DLL MoveTop_Command final : public Move_Command
{
public:
	MoveTop_Command(const _float3& vDir) :Move_Command(vDir) {};
	virtual ~MoveTop_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_TOP; }

private:
	virtual void Free() { __super::Free(); }
};

class CLIENT_DLL MoveBottom_Command final : public Move_Command
{
public:
	MoveBottom_Command(const _float3& vDir) :Move_Command(vDir) {};
	virtual ~MoveBottom_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_DOWN; }

private:
	virtual void Free() { __super::Free(); }
};

class CLIENT_DLL MoveLeft_Command final : public Move_Command
{
public:
	MoveLeft_Command(const _float3& vDir) :Move_Command(vDir) {};
	virtual ~MoveLeft_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_LEFT; }

private:
	virtual void Free() { __super::Free(); }
};

class CLIENT_DLL MoveRight_Command final : public Move_Command
{
public:
	MoveRight_Command(const _float3& vDir) :Move_Command(vDir) {};
	virtual ~MoveRight_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_RIGHT; }

private:
	virtual void Free() { __super::Free(); }
};



class CLIENT_DLL Jump_Command final : public CKirby_Command
{
public:
	virtual ~Jump_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::JUMP; }

private:
	virtual void Free() { __super::Free(); }
};


class CLIENT_DLL ATTACK_Command final : public CKirby_Command
{
public:
	virtual ~ATTACK_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::ATTACK; }

private:
	virtual void Free() { __super::Free(); }
};

NS_END