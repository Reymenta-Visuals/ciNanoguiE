/*
    nanogui/common.h -- common definitions used by NanoGUI

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "../eigen/Eigen/Core"
#include <stdint.h>
#include <array>
#include <vector>


/* Set to 1 to draw boxes around widgets */
//#define NANOGUI_SHOW_WIDGET_BOUNDS 1

#if !defined(NAMESPACE_BEGIN)
   #define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END)
   #define NAMESPACE_END(name) }
#endif

/* Forward declarations */
struct NVGcontext;
struct NVGcolor;
struct NVGglyphPosition;

NAMESPACE_BEGIN (nanogui)

// Looks like no cursor support in Cinder?
/* Cursor shapes */
enum class Cursor
{
   Arrow = 0,
   IBeam,
   Crosshair,
   Hand,
   HResize,
   VResize,
   CursorCount
};

/* Import some common Eigen types */
using Eigen::Vector2f;
using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::Vector2i;
using Eigen::Vector3i;
using Eigen::Vector4i;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::VectorXf;
using Eigen::MatrixXf;

typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;

/// Stores an RGBA color value
class Color : public Eigen::Vector4f
{
      typedef Eigen::Vector4f Base;
   public:
      Color() : Color (0, 0, 0, 0) {}

      Color (const Eigen::Vector4f & color) : Eigen::Vector4f (color) { }

      Color (const Eigen::Vector3f & color, float alpha)
         : Color (color (0), color (1), color (2), alpha) { }

      Color (const Eigen::Vector3i & color, int alpha)
         : Color (color.cast<float>() / 255.f, alpha / 255.f) { }

      Color (const Eigen::Vector3f & color) : Color (color, 1.0f) {}

      Color (const Eigen::Vector3i & color)
         : Color ((Vector3f) (color.cast<float>() / 255.f)) { }

      Color (const Eigen::Vector4i & color)
         : Color ((Vector4f) (color.cast<float>() / 255.f)) { }

      Color (float intensity, float alpha)
         : Color (Vector3f::Constant (intensity), alpha) { }

      Color (int intensity, int alpha)
         : Color (Vector3i::Constant (intensity), alpha) { }

      Color (float r, float g, float b, float a) : Color (Vector4f (r, g, b, a)) { }

      Color (int r, int g, int b, int a) : Color (Vector4i (r, g, b, a)) { }

      /// Construct a color vector from MatrixBase (needed to play nice with Eigen)
      template <typename Derived> Color (const Eigen::MatrixBase<Derived> & p)
         : Base (p) { }

      /// Assign a color vector from MatrixBase (needed to play nice with Eigen)
      template <typename Derived> Color & operator= (const Eigen::MatrixBase<Derived> & p)
      {
         this->Base::operator= (p);
         return *this;
      }

      /// Return a reference to the red channel
      float & r()
      {
         return x();
      }
      /// Return a reference to the red channel (const version)
      const float & r() const
      {
         return x();
      }
      /// Return a reference to the green channel
      float & g()
      {
         return y();
      }
      /// Return a reference to the green channel (const version)
      const float & g() const
      {
         return y();
      }
      /// Return a reference to the blue channel
      float & b()
      {
         return z();
      }
      /// Return a reference to the blue channel (const version)
      const float & b() const
      {
         return z();
      }

      Color contrastingColor() const
      {
         float luminance = cwiseProduct (Color (0.299f, 0.587f, 0.144f, 0.f)).sum();
         return Color (luminance < 0.5f ? 1.f : 0.f, 1.f);
      }

      inline operator const NVGcolor & () const
      {
         return reinterpret_cast<const NVGcolor &> (*this);
      }
};


/* Forward declarations */
template <typename T> class ref;
class AdvancedGridLayout;
class BoxLayout;
class Button;
class CheckBox;
class ColorWheel;
class ColorPicker;
class ComboBox;
class GLFramebuffer;
class GLShader;
class GridLayout;
class GroupLayout;
class ImagePanel;
class Label;
class Layout;
class MessageDialog;
class Object;
class Popup;
class PopupButton;
class ProgressBar;
class Screen;
class Slider;
class TextBox;
class Theme;
class ToolButton;
class VScrollPanel;
class Widget;
class Window;

/// Determine whether an icon ID is a texture loaded via nvgImageIcon
inline bool nvgIsImageIcon (int value)
{
   return value < 1024;
}

/// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
inline bool nvgIsFontIcon (int value)
{
   return value >= 1024;
}

inline std::array<char, 8> utf8 (int c)
{
   std::array<char, 8> seq;
   int n = 0;
   if (c < 0x80) n = 1;
   else
      if (c < 0x800) n = 2;
      else
         if (c < 0x10000) n = 3;
         else
            if (c < 0x200000) n = 4;
            else
               if (c < 0x4000000) n = 5;
               else
                  if (c <= 0x7fffffff) n = 6;
   seq[n] = '\0';
   switch (n)
   {
      case 6:
         seq[5] = 0x80 | (c & 0x3f);
         c = c >> 6;
         c |= 0x4000000;
      case 5:
         seq[4] = 0x80 | (c & 0x3f);
         c = c >> 6;
         c |= 0x200000;
      case 4:
         seq[3] = 0x80 | (c & 0x3f);
         c = c >> 6;
         c |= 0x10000;
      case 3:
         seq[2] = 0x80 | (c & 0x3f);
         c = c >> 6;
         c |= 0x800;
      case 2:
         seq[1] = 0x80 | (c & 0x3f);
         c = c >> 6;
         c |= 0xc0;
      case 1:
         seq[0] = c;
   }
   return seq;
}


// Cinder key codes
enum
{
   SHIFT_DOWN = 0x0008,
   ALT_DOWN = 0x0010,
   CTRL_DOWN = 0x0020,
   META_DOWN = 0x0040,
#if (defined( CINDER_MSW ) || defined( CINDER_WINRT ))
   ACCEL_DOWN = CTRL_DOWN
#else
   ACCEL_DOWN = META_DOWN
#endif
};

// Key codes
enum
{
   KEY_UNKNOWN = 0,
   KEY_FIRST = 0,
   KEY_BACKSPACE = 8,
   KEY_TAB = 9,
   KEY_CLEAR = 12,
   KEY_RETURN = 13,
   KEY_PAUSE = 19,
   KEY_ESCAPE = 27,
   KEY_SPACE = 32,
   KEY_EXCLAIM = 33,
   KEY_QUOTEDBL = 34,
   KEY_HASH = 35,
   KEY_DOLLAR = 36,
   KEY_AMPERSAND = 38,
   KEY_QUOTE = 39,
   KEY_LEFTPAREN = 40,
   KEY_RIGHTPAREN = 41,
   KEY_ASTERISK = 42,
   KEY_PLUS = 43,
   KEY_COMMA = 44,
   KEY_MINUS = 45,
   KEY_PERIOD = 46,
   KEY_SLASH = 47,
   KEY_0 = 48,
   KEY_1 = 49,
   KEY_2 = 50,
   KEY_3 = 51,
   KEY_4 = 52,
   KEY_5 = 53,
   KEY_6 = 54,
   KEY_7 = 55,
   KEY_8 = 56,
   KEY_9 = 57,
   KEY_COLON = 58,
   KEY_SEMICOLON = 59,
   KEY_LESS = 60,
   KEY_EQUALS = 61,
   KEY_GREATER = 62,
   KEY_QUESTION = 63,
   KEY_AT = 64,

   KEY_LEFTBRACKET = 91,
   KEY_BACKSLASH = 92,
   KEY_RIGHTBRACKET = 93,
   KEY_CARET = 94,
   KEY_UNDERSCORE = 95,
   KEY_BACKQUOTE = 96,
   KEY_a = 97,
   KEY_b = 98,
   KEY_c = 99,
   KEY_d = 100,
   KEY_e = 101,
   KEY_f = 102,
   KEY_g = 103,
   KEY_h = 104,
   KEY_i = 105,
   KEY_j = 106,
   KEY_k = 107,
   KEY_l = 108,
   KEY_m = 109,
   KEY_n = 110,
   KEY_o = 111,
   KEY_p = 112,
   KEY_q = 113,
   KEY_r = 114,
   KEY_s = 115,
   KEY_t = 116,
   KEY_u = 117,
   KEY_v = 118,
   KEY_w = 119,
   KEY_x = 120,
   KEY_y = 121,
   KEY_z = 122,
   KEY_DELETE = 127,

   KEY_KP0 = 256,
   KEY_KP1 = 257,
   KEY_KP2 = 258,
   KEY_KP3 = 259,
   KEY_KP4 = 260,
   KEY_KP5 = 261,
   KEY_KP6 = 262,
   KEY_KP7 = 263,
   KEY_KP8 = 264,
   KEY_KP9 = 265,
   KEY_KP_PERIOD = 266,
   KEY_KP_DIVIDE = 267,
   KEY_KP_MULTIPLY = 268,
   KEY_KP_MINUS = 269,
   KEY_KP_PLUS = 270,
   KEY_KP_ENTER = 271,
   KEY_KP_EQUALS = 272,

   KEY_UP = 273,
   KEY_DOWN = 274,
   KEY_RIGHT = 275,
   KEY_LEFT = 276,
   KEY_INSERT = 277,
   KEY_HOME = 278,
   KEY_END = 279,
   KEY_PAGEUP = 280,
   KEY_PAGEDOWN = 281,

   KEY_F1 = 282,
   KEY_F2 = 283,
   KEY_F3 = 284,
   KEY_F4 = 285,
   KEY_F5 = 286,
   KEY_F6 = 287,
   KEY_F7 = 288,
   KEY_F8 = 289,
   KEY_F9 = 290,
   KEY_F10 = 291,
   KEY_F11 = 292,
   KEY_F12 = 293,
   KEY_F13 = 294,
   KEY_F14 = 295,
   KEY_F15 = 296,

   KEY_NUMLOCK = 300,
   KEY_CAPSLOCK = 301,
   KEY_SCROLLOCK = 302,
   KEY_RSHIFT = 303,
   KEY_LSHIFT = 304,
   KEY_RCTRL = 305,
   KEY_LCTRL = 306,
   KEY_RALT = 307,
   KEY_LALT = 308,
   KEY_RMETA = 309,
   KEY_LMETA = 310,
   KEY_LSUPER = 311,		/* Left "Windows" key */
   KEY_RSUPER = 312,		/* Right "Windows" key */
   KEY_MODE = 313,		/* "Alt Gr" key */
   KEY_COMPOSE = 314,		/* Multi-key compose key */

   KEY_HELP = 315,
   KEY_PRINT = 316,
   KEY_SYSREQ = 317,
   KEY_BREAK = 318,
   KEY_MENU = 319,
   KEY_POWER = 320,		/* Power Macintosh power key */
   KEY_EURO = 321,		/* Some european keyboards */
   KEY_UNDO = 322,		/* Atari keyboard has Undo */

   KEY_LAST

   // mouse events
#define RELEASE                0
#define PRESS                  1
#define REPEAT				   2
#define MOUSE_BUTTON_1         0
#define MOUSE_BUTTON_2         1
#define MOUSE_BUTTON_3         2
#define MOUSE_BUTTON_LEFT      MOUSE_BUTTON_1
#define MOUSE_BUTTON_RIGHT     MOUSE_BUTTON_2
#define MOUSE_BUTTON_MIDDLE    MOUSE_BUTTON_3
};


NAMESPACE_END (nanogui)



// Define command key for windows/mac/linux
#ifdef __APPLE__
   #define SYSTEM_COMMAND_MOD nanogui::KEY_LSUPER  // FIXME just guessing here
#else
   #define SYSTEM_COMMAND_MOD nanogui::CTRL_DOWN
#endif
