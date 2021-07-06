#include <sdl_engine/sdl_input.h>

#include <string>

#include "sdl_engine/sdl_engine.h"

namespace neko::sdl
{
InputManager::InputManager() { InputLocator::provide(this); }

void InputManager::Init()
{
	/* Print information about the joysticks */
	LogDebug("There are " + std::to_string(SDL_NumJoysticks()) + " joysticks attached");
	PrintAllJoystick();

	if (SDL_NumJoysticks() > 0)
	{
		int device              = 0;
		SDL_Joystick* joystick_ = SDL_JoystickOpen(device);
		if (joystick_)
			SDL_assert(SDL_JoystickFromInstanceID(SDL_JoystickInstanceID(joystick_)) == joystick_);
	}

	for (auto& controllerInputs : controllerInputs_)
	{
		controllerInputs.controllerAxes[static_cast<size_t>(ControllerAxes::LEFT_TRIGGER)]  = -1.0f;
		controllerInputs.controllerAxes[static_cast<size_t>(ControllerAxes::RIGHT_TRIGGER)] = -1.0f;
	}
}

void InputManager::OnPreUserInput()
{
	for (size_t i = 0; i < static_cast<size_t>(KeyCode::LENGTH); i++)
	{
		if (keyboardInputs_.keyStates[i] == ButtonState::UP)
			keyboardInputs_.keyStates[i] = ButtonState::NONE;
		else if (keyboardInputs_.keyStates[i] == ButtonState::DOWN)
			keyboardInputs_.keyStates[i] = ButtonState::HELD;
	}

	for (size_t i = 0; i < static_cast<size_t>(MouseButtonType::LENGTH); i++)
	{
		if (keyboardInputs_.mouseButtonStates[i] == ButtonState::UP)
			keyboardInputs_.mouseButtonStates[i] = ButtonState::NONE;
		else if (keyboardInputs_.mouseButtonStates[i] == ButtonState::DOWN)
			keyboardInputs_.mouseButtonStates[i] = ButtonState::HELD;
	}

	for (auto& controllerInputs : controllerInputs_)
	{
		for (size_t i = 0; i < static_cast<size_t>(ControllerButtonType::LENGTH); i++)
		{
			if (controllerInputs.controllerButtonStates[i] == ButtonState::UP)
				controllerInputs.controllerButtonStates[i] = ButtonState::NONE;
			else if (controllerInputs.controllerButtonStates[i] == ButtonState::DOWN)
				controllerInputs.controllerButtonStates[i] = ButtonState::HELD;
		}
	}

	mouseScroll_ = Vec2f::zero;
}

unsigned InputManager::FindControllerIndexFromId(const JoystickId controllerId) const
{
	const auto controllerInputIt = std::find_if(controllerInputs_.begin(),
		controllerInputs_.end(),
		[controllerId](ControllerInputs controllerInputs)
		{ return controllerInputs.controllerId == controllerId; });

	if (controllerInputIt >= controllerInputs_.end())
	{
		LogDebug("Invalid controllerId : " + std::to_string(controllerId));
		return controllerInputs_.size();
	}

	const unsigned index = std::distance(controllerInputs_.begin(), controllerInputIt);
	return index;
}

unsigned InputManager::FindControllerIndexFromPlayerId(JoyPlayerId controllerId) const
{
	const auto controllerInputIt = std::find_if(controllerInputs_.begin(),
		controllerInputs_.end(),
		[controllerId](ControllerInputs controllerInputs)
		{ return controllerInputs.controllerPlayerId == controllerId; });
	if (controllerInputIt >= controllerInputs_.end()) { return controllerInputs_.size(); }
	const unsigned index = std::distance(controllerInputs_.begin(), controllerInputIt);
	return index;
}

void InputManager::OnEvent(SDL_Event event)
{
	switch (event.type)
	{
		case SDL_TEXTINPUT:
		{
			// TODO(@Luca) Setup Text Input
			break;
		}
		case SDL_TEXTEDITING:
		{
			// TODO(@Luca) Setup Text Editing
			break;
		}

		case SDL_KEYDOWN:
		{
			const size_t index = event.key.keysym.scancode;
			if (keyboardInputs_.keyStates[index] != ButtonState::HELD)
				keyboardInputs_.keyStates[index] = ButtonState::DOWN;
			break;
		}
		case SDL_KEYUP:
		{
			const size_t index               = event.key.keysym.scancode;
			keyboardInputs_.keyStates[index] = ButtonState::UP;
			break;
		}

		// Mouse
		case SDL_MOUSEMOTION:
		{
			mousePos_.x         = static_cast<float>(event.motion.x);
			mousePos_.y         = static_cast<float>(event.motion.y);
			mouseRelativePos_.x = static_cast<float>(event.motion.xrel);
			mouseRelativePos_.y = static_cast<float>(event.motion.yrel);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			const size_t index                       = event.button.button - 1;
			keyboardInputs_.mouseButtonStates[index] = ButtonState::DOWN;

			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			const size_t index                       = event.button.button - 1;
			keyboardInputs_.mouseButtonStates[index] = ButtonState::UP;

			break;
		}
		case SDL_MOUSEWHEEL:
		{
			mouseScroll_.x = static_cast<float>(event.wheel.x);
			mouseScroll_.y = static_cast<float>(event.wheel.y);
			break;
		}

        // Joystick
		case SDL_JOYDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			LogDebug("Joystick device  removed." +
					 std::to_string(static_cast<int>(event.cdevice.which)));
			SDL_Joystick* joystick_ = SDL_JoystickFromInstanceID(event.cdevice.which);
			if (joystick_ != nullptr)
			{
				LogDebug("Our instance ID is" +
						 std::to_string(static_cast<int>(SDL_JoystickInstanceID(joystick_))));
				LogDebug("There are " + std::to_string(SDL_NumJoysticks()) + " joysticks attached");
				SDL_JoystickClose(joystick_);
				SDL_GameControllerClose(SDL_GameControllerFromInstanceID(event.cdevice.which));
			}
			else
			{
				LogDebug("Joystick removed not found");
			}
			const unsigned controllerIndex = FindControllerIndexFromId(event.cdevice.which);
			if (controllerIndex < controllerInputs_.size())
			{
				controllerInputs_.erase(controllerInputs_.begin() + controllerIndex);
			}
			else
			{
				LogDebug(&"Controller removed out of range : Joystick "[event.cdevice.which]);
			}

			break;
		}
		case SDL_JOYDEVICEADDED:
		case SDL_CONTROLLERDEVICEADDED:
		{
			const int device        = event.jdevice.which;
			SDL_Joystick* joystick_ = SDL_JoystickOpen(device);
			SDL_GameControllerOpen(device);
			LogDebug("Joystick device added." +
					 std::to_string(static_cast<int>(SDL_JoystickInstanceID(joystick_))));
			if (joystick_ != nullptr)
			{
				SDL_assert(
					SDL_JoystickFromInstanceID(SDL_JoystickInstanceID(joystick_)) == joystick_);
			}
			LogDebug("There are " + std::to_string(SDL_NumJoysticks()) + " joysticks attached");
			const unsigned controllerId    = SDL_JoystickInstanceID(joystick_);
			const unsigned controllerIndex = FindControllerIndexFromId(controllerId);
			if (controllerIndex >= controllerInputs_.size())
			{
				ControllerInputs controllerInputs;
				controllerInputs.controllerId       = controllerId;
				controllerInputs.controllerPlayerId = SDL_JoystickGetPlayerIndex(joystick_);
				controllerInputs_.push_back(controllerInputs);
			}
			else
			{
				LogDebug(&"Controller already added : Joystick "[controllerId]);
			}
			break;
		}

		case SDL_JOYBALLMOTION:
		{
			LogDebug("Joystick " + std::to_string(event.jball.which) + " ball " +
					 std::to_string(event.jball.ball) + " delta: (" +
					 std::to_string(event.jball.xrel) + "," + std::to_string(event.jball.yrel) +
					 ")");
			LogDebug("JoyBall not support");

			break;
		}

		case SDL_CONTROLLERBUTTONDOWN:
		{
			const unsigned controllerIndex = FindControllerIndexFromId(event.cdevice.which);
			if (controllerIndex < controllerInputs_.size())
			{
				controllerInputs_[controllerIndex].controllerButtonStates[event.cbutton.button] =
					ButtonState::DOWN;
			}
			else
			{
				LogDebug(&"JoyButton pressed down from unknown Joystick : Joystick "[event.cdevice
																						 .which]);
			}
			break;
		}

		case SDL_CONTROLLERBUTTONUP:
		{
			const unsigned controllerIndex = FindControllerIndexFromId(event.cdevice.which);
			if (controllerIndex < controllerInputs_.size())
			{
				controllerInputs_[controllerIndex].controllerButtonStates[event.cbutton.button] =
					ButtonState::UP;
			}
			else
			{
				LogDebug(
					&"JoyButton released up from unknow Joystick : Joystick "[event.cdevice.which]);
			}
			break;
		}

		case SDL_CONTROLLERAXISMOTION:
		{
			const int deadZone = 3200;
			float value        = 0;
			if (event.caxis.value < -deadZone || event.caxis.value > deadZone)
			{
				value = static_cast<float>(event.caxis.value) / kMaxJoyValue_;
			}
			const unsigned controllerIndex = FindControllerIndexFromId(event.caxis.which);
			if (controllerIndex < controllerInputs_.size())
			{
				controllerInputs_[controllerIndex].controllerAxes[event.caxis.axis] = value;
			}
			else
			{
				LogDebug(&"JoyAxis from unknown Joystick : Joystick "[event.cdevice.which]);
			}
			break;
		}

			/* Fall through to signal quit */
		case SDL_FINGERDOWN:
		{
			LogDebug("Finger down");
			break;
		}
		case SDL_FINGERMOTION:
		{
			LogDebug("Finger motion");
			break;
		}
		case SDL_FINGERUP:
		{
			LogDebug("Finger up");
			break;
		}

		default: break;
	}
}

ButtonState InputManager::GetKeyState(KeyCode key) const
{
	return keyboardInputs_.keyStates[static_cast<size_t>(key)];
}

bool InputManager::IsKeyDown(KeyCode key) const
{
	return GetKeyState(key) == ButtonState::DOWN;
}

bool InputManager::IsKeyHeld(KeyCode key) const
{
    return GetKeyState(key) == ButtonState::HELD;
}

bool InputManager::IsKeyUp(KeyCode key) const
{
    return GetKeyState(key) == ButtonState::UP;
}

ButtonState InputManager::GetControllerButtonState(
	JoyPlayerId controllerId, ControllerButtonType controllerButton) const
{
	const unsigned controllerIndex = FindControllerIndexFromPlayerId(controllerId);
	if (controllerIndex >= controllerInputs_.size()) { return ButtonState::NONE; }
	return controllerInputs_[controllerIndex]
	    .controllerButtonStates[static_cast<size_t>(controllerButton)];
}

float InputManager::GetControllerAxis(JoyPlayerId controllerId, ControllerAxes axis) const
{
	const unsigned controllerIndex = FindControllerIndexFromPlayerId(controllerId);
	if (controllerIndex >= controllerInputs_.size()) { return 0.0f; }
	return controllerInputs_[controllerIndex].controllerAxes[static_cast<size_t>(axis)];
}

std::vector<JoyPlayerId> InputManager::GetControllerIdVector() const
{
	std::vector<JoyPlayerId> controllerIdVector;
	for (const ControllerInputs& controllerInput : controllerInputs_)
	{
		controllerIdVector.push_back(controllerInput.controllerPlayerId);
	}
	return controllerIdVector;
}

void InputManager::PrintJoystick(const int device)
{
	//Print info
	LogDebug("Joystick DeviceId: " + std::to_string(device));
	LogDebug("Controller : " + std::to_string(SDL_IsGameController(device)));
	const auto joystick = SDL_JoystickOpen(device);
	if (joystick)
	{
		std::string type;

		switch (SDL_JoystickGetType(joystick))
		{
			case SDL_JOYSTICK_TYPE_GAMECONTROLLER: type = "Game Controller"; break;
			case SDL_JOYSTICK_TYPE_WHEEL: type = "Wheel"; break;
			case SDL_JOYSTICK_TYPE_ARCADE_STICK: type = "Arcade Stick"; break;
			case SDL_JOYSTICK_TYPE_FLIGHT_STICK: type = "Flight Stick"; break;
			case SDL_JOYSTICK_TYPE_DANCE_PAD: type = "Dance Pad"; break;
			case SDL_JOYSTICK_TYPE_GUITAR: type = "Guitar"; break;
			case SDL_JOYSTICK_TYPE_DRUM_KIT: type = "Drum Kit"; break;
			case SDL_JOYSTICK_TYPE_ARCADE_PAD: type = "Arcade Pad"; break;
			case SDL_JOYSTICK_TYPE_THROTTLE: type = "Throttle"; break;
			default: type = "Unknown"; break;
		}

		LogDebug("Name: " + std::string(SDL_JoystickName(joystick)));
		LogDebug("Instance id: " + std::to_string(SDL_JoystickInstanceID(joystick)));
		LogDebug("Player id: " + std::to_string(SDL_JoystickGetPlayerIndex(joystick)));

		LogDebug("Power level: " +
				 std::to_string(static_cast<int>(SDL_JoystickCurrentPowerLevel(joystick))));
		LogDebug("Type: " + type);
		LogDebug("Axes: " + std::to_string(SDL_JoystickNumAxes(joystick)));
		LogDebug("Balls: " + std::to_string(SDL_JoystickNumBalls(joystick)));
		LogDebug("Hats: " + std::to_string(SDL_JoystickNumHats(joystick)));
		LogDebug("Buttons: " + std::to_string(SDL_JoystickNumButtons(joystick)));

		SDL_JoystickClose(joystick);
	}
	else
	{
		LogDebug("SDL_JoystickOpen " + std::to_string(device) + " failed");
	}
}

void InputManager::PrintAllJoystick()
{
	LogDebug("-----------Start Print All-----------");
	unsigned nbJoystick = SDL_NumJoysticks();
	LogDebug("Nb Joystick : " + std::to_string(nbJoystick));
	for (int deviceId = 0; deviceId < SDL_NumJoysticks(); ++deviceId) PrintJoystick(deviceId);

	LogDebug("-------------End Print All-----------");
}

std::string InputManager::PcInputsEnumToString(const KeyCode keyCode)
{
	switch (keyCode)
	{
		case KeyCode::A: return "A";
		case KeyCode::B: return "B";
		case KeyCode::C: return "C";
		case KeyCode::D: return "D";
		case KeyCode::E: return "E";
		case KeyCode::F: return "F";
		case KeyCode::G: return "G";
		case KeyCode::H: return "H";
		case KeyCode::I: return "I";
		case KeyCode::J: return "J";
		case KeyCode::K: return "K";
		case KeyCode::L: return "L";
		case KeyCode::M: return "M";
		case KeyCode::N: return "N";
		case KeyCode::O: return "O";
		case KeyCode::P: return "P";
		case KeyCode::Q: return "Q";
		case KeyCode::R: return "R";
		case KeyCode::S: return "S";
		case KeyCode::T: return "T";
		case KeyCode::U: return "U";
		case KeyCode::V: return "V";
		case KeyCode::W: return "W";
		case KeyCode::X: return "X";
		case KeyCode::Y: return "Y";
		case KeyCode::Z: return "Z";
		case KeyCode::ESCAPE: return "Escape";
		case KeyCode::SPACE: return "Space";
		case KeyCode::KEY_LEFT_SHIFT: return "Left_Shift";
		case KeyCode::KEY_RIGHT_SHIFT: return "Right_Shift";
		case KeyCode::KEY_LEFT_CTRL: return "Left_Ctrl";
		case KeyCode::KEY_RIGHT_CTRL: return "Right_Ctrl";
		case KeyCode::KEY_LEFT_ALT: return "Left_Alt";
		case KeyCode::LEFT: return "Left";
		case KeyCode::RIGHT: return "Right";
		case KeyCode::UP: return "Up";
		case KeyCode::DOWN: return "Down";
		default: return "Unknown";
	}
}

std::string InputManager::MouseInputsEnumToString(MouseButtonType mouseButton)
{
	switch (mouseButton)
	{
		case MouseButtonType::LEFT: return "LeftMouse";
		case MouseButtonType::RIGHT: return "RightMouse";
		case MouseButtonType::MIDDLE: return "MiddleMouse";
		default: return "";
	}
}

std::string InputManager::ControllerInputsEnumToString(const ControllerButtonType controller)
{
	return SDL_GameControllerGetStringForButton(SDL_GameControllerButton(controller));
}

std::string InputManager::ControllerInputsEnumToString(const ControllerAxes controller)
{
	return SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis(controller));
}
}    // namespace neko::sdl
