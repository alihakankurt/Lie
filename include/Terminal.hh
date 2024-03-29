#pragma once

#include <Core.hh>
#include <Buffer.hh>

#if defined(LIE_UNIX)
#    include <unistd.h>
#    include <termios.h>
#    include <sys/ioctl.h>
#endif

namespace Lie
{
    enum class Key
    {
        Null = 0,
        CtrlA = 1,
        CtrlB = 2,
        CtrlC = 3,
        Interrupt = CtrlC,
        CtrlD = 4,
        EndOfFile = CtrlD,
        CtrlE = 5,
        CtrlF = 6,
        CtrlG = 7,
        CtrlH = 8,
        Backspace = CtrlH,
        CtrlI = 9,
        Tab = CtrlI,
        CtrlJ = 10,
        CtrlK = 11,
        CtrlL = 12,
        FormFeed = CtrlL,
        CtrlM = 13,
        Enter = CtrlM,
        CtrlN = 14,
        CtrlO = 15,
        CtrlP = 16,
        CtrlQ = 17,
        CtrlR = 18,
        CtrlS = 19,
        Pause = CtrlS,
        CtrlT = 20,
        CtrlU = 21,
        CtrlV = 22,
        CtrlW = 23,
        CtrlX = 24,
        CtrlY = 25,
        CtrlZ = 26,
        CtrlLeftBracket = 27,
        Escape = CtrlLeftBracket,
        CtrlBackslash = 28,
        CtrlRightBracket = 29,
        CtrlCaret = 30,
        CtrlUnderscore = 31,
        Space = 32,
        ExclamationMark = 33,
        DoubleQuote = 34,
        Hash = 35,
        Dollar = 36,
        Percent = 37,
        Ampersand = 38,
        SingleQuote = 39,
        LeftParenthesis = 40,
        RightParenthesis = 41,
        Asterisk = 42,
        Plus = 43,
        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,
        Zero = 48,
        One = 49,
        Two = 50,
        Three = 51,
        Four = 52,
        Five = 53,
        Six = 54,
        Seven = 55,
        Eight = 56,
        Nine = 57,
        Colon = 58,
        Semicolon = 59,
        LessThan = 60,
        Equal = 61,
        GreaterThan = 62,
        QuestionMark = 63,
        At = 64,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LeftBracket = 91,
        Backslash = 92,
        RightBracket = 93,
        Caret = 94,
        Underscore = 95,
        Backquote = 96,
        LowerA = 97,
        LowerB = 98,
        LowerC = 99,
        LowerD = 100,
        LowerE = 101,
        LowerF = 102,
        LowerG = 103,
        LowerH = 104,
        LowerI = 105,
        LowerJ = 106,
        LowerK = 107,
        LowerL = 108,
        LowerM = 109,
        LowerN = 110,
        LowerO = 111,
        LowerP = 112,
        LowerQ = 113,
        LowerR = 114,
        LowerS = 115,
        LowerT = 116,
        LowerU = 117,
        LowerV = 118,
        LowerW = 119,
        LowerX = 120,
        LowerY = 121,
        LowerZ = 122,
        LeftCurlyBrace = 123,
        Pipe = 124,
        RightCurlyBrace = 125,
        Tilde = 126,
        Delete = 127,
        Up = 128,
        Down = 129,
        Right = 130,
        Left = 131,
        PageUp = 132,
        PageDown = 133,
        Home = 134,
        End = 135,
        Insert = 136,
    };

    class Terminal
    {
      public:
        static auto EnableRawMode() -> void;
        static auto DisableRawMode() -> void;

        [[nodiscard]] static auto GetWindowSize() -> Size;

        [[nodiscard]] static auto Read() -> Key;
        static auto Write(const char* data, int size) -> void;
        static auto Flush() -> bool;

        static auto ClearScreen() -> void;
        static auto ClearLine() -> void;
        static auto MoveCursor(int x, int y) -> void;
        static auto HideCursor() -> void;
        static auto ShowCursor() -> void;

      private:
        static Buffer buffer;

#if defined(LIE_UNIX)
        static termios original;
#endif
    };
}
