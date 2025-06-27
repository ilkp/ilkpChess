#include "input.h"

#include <algorithm>
#include <functional>
#include <iostream>

Input::Input() :
	_mousePosition({ 0, 0 })
{
}

Key Input::getKey(SDL_Keycode keycode) const
{
	const auto& current = _keyMap.ccurrent();
	if (current.count(keycode) == 0)
		return Key{ .direction = KeyDirection::up, .status = KeyStatus::held };
	return current.at(keycode);
}

Key Input::getMouseButton(SDL_MouseButtonFlags button) const
{
	const auto& current = _mouseMap.ccurrent();
	if (current.count(button) == 0)
		return Key{ .direction = KeyDirection::up, .status = KeyStatus::held };
	return current.at(button);
}

Vec2f Input::getMousePosition() const
{
	return _mousePosition;
}

void Input::updateKeyboardMaps()
{
	auto& current = _keyMap.current();
	auto& previous = _keyMap.previous();
	std::map<SDL_Keycode, Key> tempKeys{};
	std::swap(tempKeys, previous);
	for (auto& [keycode, key] : current)
	{
		if (tempKeys.count(keycode) > 0 && tempKeys[keycode].direction == key.direction)
			key.status = KeyStatus::held;
		previous.insert({ keycode, Key{.direction = key.direction, .status = key.status } });
	}
}

void Input::updateMouseMaps()
{
	auto& current = _mouseMap.current();
	auto& previous = _mouseMap.previous();
	std::map<SDL_MouseButtonFlags, Key> tempKeys{};
	std::swap(tempKeys, previous);
	for (auto& [keycode, key] : current)
	{
		if (tempKeys.count(keycode) > 0 && tempKeys[keycode].direction == key.direction)
			key.status = KeyStatus::held;
		previous.insert({ keycode, Key{.direction = key.direction, .status = key.status } });
	}
}

void Input::onSdlEvent(SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_EventType::SDL_EVENT_KEY_DOWN:
	{
		_keyMap.current()[event->key.key] = { .direction = KeyDirection::down, .status = KeyStatus::begin };
		break;
	}
	case SDL_EventType::SDL_EVENT_KEY_UP:
	{
		_keyMap.current()[event->key.key] = { .direction = KeyDirection::up, .status = KeyStatus::begin };
		break;
	}
	}
}

void Input::onSdlMouseButtonEvent(SDL_Event* event)
{
	const SDL_MouseButtonFlags button = event->button.button;
	switch (event->button.type)
	{
	case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
	{
		_mouseMap.current()[button] = { .direction = KeyDirection::down, .status = KeyStatus::begin };
		break;
	}
	case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
	{
		_mouseMap.current()[button] = { .direction = KeyDirection::up, .status = KeyStatus::begin };
		break;
	}
	}
}

void Input::onSdlMouseMotionEvent(SDL_Event* event)
{
	_mousePosition.x = event->motion.x;
	_mousePosition.y = event->motion.y;
}

void Input::onSdlIterate()
{
	updateKeyboardMaps();
	updateMouseMaps();
}