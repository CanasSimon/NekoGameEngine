#include <sdl_engine/sdl_input.h>

#include <string>

#include "sdl_engine/sdl_engine.h"

#ifdef NN_NINTENDO_SDK
#include <nn/hid/hid_ControllerSupport.h>
#endif

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

		if (joystick_ != nullptr)
		{
			SDL_assert(SDL_JoystickFromInstanceID(SDL_JoystickInstanceID(joystick_)) == joystick_);
		}
	}

	for (auto& controllerInputs : controllerInputs_)
	{
		controllerInputs.controllerAxis[static_cast<size_t>(ControllerAxes::LEFT_BUMPER)] =
			-1.0f;
		controllerInputs.controllerAxis[static_cast<size_t>(ControllerAxes::RIGHT_BUMPER)] =
			-1.0f;
	}
}

void InputManager::OnPreUserInput()
{
	for (size_t i = 0; i < static_cast<size_t>(KeyCodeType::KEYBOARD_SIZE); i++)
	{
		if (keyPressedState_[i] == ButtonState::UP) keyPressedState_[i] = ButtonState::NONE;
		else if (keyPressedState_[i] == ButtonState::DOWN)
			keyPressedState_[i] = ButtonState::HELD;
	}

	for (size_t i = 0; i < static_cast<size_t>(MouseButtonType::LENGTH); i++)
	{
		if (buttonState_[i] == ButtonState::UP) buttonState_[i] = ButtonState::NONE;
		else if (buttonState_[i] == ButtonState::DOWN)
			buttonState_[i] = ButtonState::HELD;
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

unsigned InputManager::FindSwitchIndexFromId(const JoystickId switchJoyId) const
{
	const auto switchInputIt = std::find_if(switchInputs_.begin(),
		switchInputs_.end(),
		[switchJoyId](SwitchInputs switchInputs)
		{ return switchInputs.switchJoyInstanceId == switchJoyId; });
	if (switchInputIt >= switchInputs_.end())
	{
		LogDebug("Invalid switchJoyInstanceId : " + std::to_string(switchJoyId));
		return switchInputs_.size();
	}
	const unsigned index = std::distance(switchInputs_.begin(), switchInputIt);
	return index;
}

unsigned InputManager::FindSwitchIndexFromPlayerId(JoyPlayerId switchJoyId) const
{
	const auto switchInputIt = std::find_if(switchInputs_.begin(),
		switchInputs_.end(),
		[switchJoyId](SwitchInputs switchInputs)
		{ return switchInputs.switchJoyPlayerId == switchJoyId; });
	if (switchInputIt >= switchInputs_.end())
	{
		LogDebug("Invalid switchJoyPlayerId : " + std::to_string(switchJoyId));
		return switchInputs_.size();
	}
	const unsigned index = std::distance(switchInputs_.begin(), switchInputIt);
	return index;
}

void InputManager::OnEvent(SDL_Event event)
{
	switch (event.type)
	{
#pragma region keyboard
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
			if (keyPressedState_[index] != ButtonState::HELD)
			{
				keyPressedState_[index] = ButtonState::DOWN;
			}
			break;
		}
		case SDL_KEYUP:
		{
			const size_t index      = event.key.keysym.scancode;
			keyPressedState_[index] = ButtonState::UP;
			break;
		}
#pragma endregion

#pragma region Mouse
		case SDL_MOUSEMOTION:
		{
			mousePos_.x         = event.motion.x;
			mousePos_.y         = event.motion.y;
			mouseRelativePos_.x = event.motion.xrel;
			mouseRelativePos_.y = event.motion.yrel;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			const size_t index  = event.button.button - 1;
			buttonState_[index] = ButtonState::DOWN;

			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			const size_t index  = event.button.button - 1;
			buttonState_[index] = ButtonState::UP;

			break;
		}
		case SDL_MOUSEWHEEL:
		{
			mouseScroll_.x = event.wheel.x;
			mouseScroll_.y = event.wheel.y;
			break;
		}
#pragma endregion

#pragma region Joystick
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
				controllerInputs_[controllerIndex].controllerAxis[event.caxis.axis] = value;
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
#pragma endregion

		default: break;
	}
}

ButtonState InputManager::GetKeyState(KeyCodeType key) const
{
	return keyPressedState_[static_cast<size_t>(key)];
}

ButtonState InputManager::GetSwitchButtonState(
	JoyPlayerId switchJoyId, SwitchButtonType switchButton) const
{
	const unsigned switchIndex = FindSwitchIndexFromPlayerId(switchJoyId);
	if (switchIndex >= switchInputs_.size())
	{
		LogDebug("Unknown Joystick : " + std::to_string(switchJoyId));
		return ButtonState::NONE;
	}
	return switchInputs_[switchIndex].switchButtonStates[static_cast<size_t>(switchButton)];
}

float InputManager::GetSwitchAxis(JoyPlayerId switchJoyId, SwitchAxisType axis) const
{
	const unsigned switchIndex = FindSwitchIndexFromPlayerId(switchJoyId);
	if (switchIndex >= switchInputs_.size())
	{
		LogDebug("Unknown Joystick : " + std::to_string(switchJoyId));
		return 0.0f;
	}
	return switchInputs_[switchIndex].switchAxis[static_cast<size_t>(axis)];
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
	return controllerInputs_[controllerIndex].controllerAxis[static_cast<size_t>(axis)];
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

std::vector<JoyPlayerId> InputManager::GetSwitchJoyIdVector() const
{
	std::vector<JoyPlayerId> switchJoyIdVector;
	for (const SwitchInputs& switchInput : switchInputs_)
	{
		switchJoyIdVector.push_back(switchInput.switchJoyPlayerId);
	}
	return switchJoyIdVector;
}

void InputManager::PrintJoystick(const int device)
{
	//Print info
	LogDebug("Joystick DeviceId: " + std::to_string(device));
	LogDebug("Controller : " + std::to_string(SDL_IsGameController(device)));
	const auto joystick = SDL_JoystickOpen(device);
	if (joystick == nullptr) { LogDebug("SDL_JoystickOpen " + std::to_string(device) + " failed"); }
	else
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
}

void InputManager::PrintAllJoystick()
{
	LogDebug("-----------Start Print All-----------");
	unsigned nbJoystick = SDL_NumJoysticks();
	LogDebug("Nb Joystick : " + std::to_string(nbJoystick));
	for (int deviceId = 0; deviceId < SDL_NumJoysticks(); ++deviceId)
	{
		PrintJoystick(deviceId);
	}

	LogDebug("-------------End Print All-----------");
}

std::string InputManager::PcInputsEnumToString(const KeyCodeType keyCode)
{
	switch (keyCode)
	{
		case KeyCodeType::A: return "A";
		case KeyCodeType::B: return "B";
		case KeyCodeType::C: return "C";
		case KeyCodeType::D: return "D";
		case KeyCodeType::E: return "E";
		case KeyCodeType::F: return "F";
		case KeyCodeType::G: return "G";
		case KeyCodeType::H: return "H";
		case KeyCodeType::I: return "I";
		case KeyCodeType::J: return "J";
		case KeyCodeType::K: return "K";
		case KeyCodeType::L: return "L";
		case KeyCodeType::M: return "M";
		case KeyCodeType::N: return "N";
		case KeyCodeType::O: return "O";
		case KeyCodeType::P: return "P";
		case KeyCodeType::Q: return "Q";
		case KeyCodeType::R: return "R";
		case KeyCodeType::S: return "S";
		case KeyCodeType::T: return "T";
		case KeyCodeType::U: return "U";
		case KeyCodeType::V: return "V";
		case KeyCodeType::W: return "W";
		case KeyCodeType::X: return "X";
		case KeyCodeType::Y: return "Y";
		case KeyCodeType::Z: return "Z";
		case KeyCodeType::ESCAPE: return "Escape";
		case KeyCodeType::SPACE: return "Space";
		case KeyCodeType::KEY_LEFT_SHIFT: return "Left_Shift";
		case KeyCodeType::KEY_RIGHT_SHIFT: return "Right_Shift";
		case KeyCodeType::KEY_LEFT_CTRL: return "Left_Ctrl";
		case KeyCodeType::KEY_RIGHT_CTRL: return "Right_Ctrl";
		case KeyCodeType::KEY_LEFT_ALT: return "Left_Alt";
		case KeyCodeType::LEFT: return "Left";
		case KeyCodeType::RIGHT: return "Rightt";
		case KeyCodeType::UP: return "Up";
		case KeyCodeType::DOWN: return "Down";
		default: return "Not register key";
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
std::string InputManager::SwitchInputsEnumToString(const SwitchButtonType switchInputs)
{
	switch (switchInputs)
	{
		default: return "";
	}
}

std::string InputManager::SwitchInputsEnumToString(SwitchAxisType switchAxis)
{
	switch (switchAxis)
	{
		default: return "";
	}
}

}    // namespace neko::sdl
