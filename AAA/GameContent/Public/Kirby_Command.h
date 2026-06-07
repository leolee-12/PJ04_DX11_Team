#pragma once

#include "GameContent_Defines.h"

NS_BEGIN(Client)

enum class KIRBY_COMMAND_TYPE
{
	NONE,
	MOVE_TOP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
	JUMP,  // A
	ATTACK // B

};

class CLIENT_DLL CKirby_Command abstract
{
public:
	virtual ~CKirby_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() = 0;
};

class CLIENT_DLL MoveTop_Command final : public CKirby_Command
{
public:
	virtual ~MoveTop_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_TOP; }
};

class CLIENT_DLL MoveBottom_Command final : public CKirby_Command
{
public:
	virtual ~MoveBottom_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_LEFT; }
};

class CLIENT_DLL MoveLeft_Command final : public CKirby_Command
{
public:
	virtual ~MoveLeft_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_LEFT; }
};

class CLIENT_DLL MoveRight_Command final : public CKirby_Command
{
public:
	virtual ~MoveRight_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::MOVE_RIGHT; }
};


class CLIENT_DLL Jump_Command final : public CKirby_Command
{
public:
	virtual ~Jump_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::JUMP; }
};


class CLIENT_DLL ATTACK_Command final : public CKirby_Command
{
public:
	virtual ~ATTACK_Command() = default;
	virtual KIRBY_COMMAND_TYPE GetCommandType() override { return KIRBY_COMMAND_TYPE::ATTACK; }
};

NS_END