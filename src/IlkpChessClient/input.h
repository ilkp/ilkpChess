#pragma once

#include "math_types.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

#include <map>
#include <array>
#include <set>

enum class KeyDirection
{
	up,
	down
};

enum class KeyStatus
{
	begin,
	held
};

struct Key
{
	KeyDirection direction;
	KeyStatus status;
};

class KeyboardMap
{
public:
	std::map<SDL_Keycode, Key>& current() { return _maps[0]; }
	std::map<SDL_Keycode, Key>& previous() { return _maps[1]; }
	const std::map<SDL_Keycode, Key>& ccurrent() const { return _maps[0]; }
	const std::map<SDL_Keycode, Key>& cprevious() const { return _maps[1]; }

private:
	std::array<std::map<SDL_Keycode, Key>, 2> _maps;
};

class MouseButtonMap
{
public:
	std::map<SDL_MouseButtonFlags, Key>& current() { return _maps[0]; }
	std::map<SDL_MouseButtonFlags, Key>& previous() { return _maps[1]; }
	const std::map<SDL_MouseButtonFlags, Key>& ccurrent() const { return _maps[0]; }
	const std::map<SDL_MouseButtonFlags, Key>& cprevious() const { return _maps[1]; }

private:
	std::array<std::map<SDL_MouseButtonFlags, Key>, 2> _maps;
};

class Input
{
public:
	Input();

	Key getKey(SDL_Keycode keycode) const;
	Key getMouseButton(SDL_MouseButtonFlags button) const;
	Vec2f getMousePosition() const;
	void onSdlEvent(SDL_Event* event);
	void onSdlIterate();

private:
	KeyboardMap _keyMap;
	MouseButtonMap _mouseMap;
	Vec2f _mousePosition;

	void updateKeyboardMaps();
	void updateMouseMaps();
	void onSdlMouseButtonEvent(SDL_Event* event);
	void onSdlMouseMotionEvent(SDL_Event* event);
};