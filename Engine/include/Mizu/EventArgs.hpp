// with help of https://github.com/jpvanoosten/LearningDirectX12/blob/v0.0.2/DX12Lib/inc/Events.h
#pragma once
#include "KeyCodes.hpp"

namespace Mizu
{
    struct EventArgs
    {
    };

    struct ResizeEventArgs : public EventArgs
    {
        typedef EventArgs base;
        ResizeEventArgs(int width, int height)
            : width(width)
            , height(height)
        {
        }

        int width;
        int height;
    };

    struct UpdateEventArgs : public EventArgs
    {
        UpdateEventArgs(double deltaTime, double totalTime)
            : elapsedTime(deltaTime)
            , totalTime(totalTime)
        {
        }

        double elapsedTime;
        double totalTime;
    };

    struct RenderEventArgs : public EventArgs
    {
        RenderEventArgs(float fDeltaTime, float fTotalTime)
            : elapsedTime(fDeltaTime)
            , totalTime(fTotalTime)
        {
        }

        float elapsedTime;
        float totalTime;
    };

    struct KeyEventArgs : EventArgs
    {
        enum keyState
        {
            Released = 0,
            Pressed = 1
        };

        KeyEventArgs(KeyCode key, unsigned int c, keyState state, bool ctrl, bool shift, bool alt)
            : key(key)
            , charKey(c)
            , state(state)
            , ctrl(ctrl)
            , shift(shift)
            , alt(alt)
        {
        }

        KeyCode key;
        unsigned int charKey;
        keyState state;
        bool ctrl;
        bool shift;
        bool alt;
    };

    struct MouseButtonEventArgs : EventArgs
    {
        enum MouseButton
        {
            None = 0,
            Left = 1,
            Right = 2,
            Middel = 3
        };
        enum ButtonState
        {
            Released = 0,
            Pressed = 1
        };

        typedef EventArgs base;
        MouseButtonEventArgs(MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool ctrl, bool shift, int x, int y)
            : button(buttonID)
            , state(state)
            , leftButton(leftButton)
            , middleButton(middleButton)
            , rightButton(rightButton)
            , ctrl(ctrl)
            , Shift(shift)
            , x(x)
            , y(y)
        {
        }

        MouseButton button; // The mouse button that was pressed or released.
        ButtonState state; // Was the button pressed or released?
        bool leftButton; // Is the left mouse button down?
        bool middleButton; // Is the middle mouse button down?
        bool rightButton; // Is the right mouse button down?
        bool ctrl; // Is the CTRL key down?
        bool Shift; // Is the shift key down?

        int x; // The x-position of the cursor relative to the upper-left corner of the client area.
        int y; // The y-position of the cursor relative to the upper-left corner of the client area.
        
    };

    struct MouseMotionEventArgs : EventArgs
    {
        MouseMotionEventArgs(bool leftButton,
            bool rightButton,
            bool middleButton,
            bool ctrl,
            bool shift, int x, int y)
            : leftButton()
            , rightButton()
            , middleButton()
            , ctrl()
            , shift()
            , x(x)
            , y(y)
        {
        }

        bool leftButton;
        bool rightButton;
        bool middleButton;
        bool ctrl;
        bool shift;

        int x;
        int y;
        int dx; // how far the mouse moved on x axis since the last event
        int dy; // how far the mouse moved on y axis since the last event
    };

    struct MouseWheelEventArgs : EventArgs
    {
        MouseWheelEventArgs(float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y)
            : wheelDelta(wheelDelta)
            , leftButton(leftButton)
            , middleButton(middleButton)
            , rightButton(rightButton)
            , control(control)
            , shift(shift)
            , x(x)
            , y(y)
        {
        }

        float wheelDelta; // How much the mouse wheel has moved. A positive value indicates that the wheel was moved to the right. A negative value indicates the wheel was moved to the left.
        bool leftButton; // Is the left mouse button down?
        bool middleButton; // Is the middle mouse button down?
        bool rightButton; // Is the right mouse button down?
        bool control; // Is the CTRL key down?
        bool shift; // Is the shift key down?

        int x; // The x-position of the cursor relative to the upper-left corner of the client area.
        int y; // The y-position of the cursor relative to the upper-left corner of the client area.
    };

    // TODO I don't know what is this for right now but will check it out soon
    struct UserEventArgs : public EventArgs
    {
        UserEventArgs(int code, void* data1, void* data2)
            : code(code)
            , data1(data1)
            , data2(data2)
        {
        }

        int code;
        void* data1;
        void* data2;
    };
}