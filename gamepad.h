#ifndef GAMEPAD
#define GAMEPAD

#include <iostream>
#include <dinput.h>
#include <string>

class Gamepad
{
    private:
        LPDIRECTINPUT8 di;
        LPDIRECTINPUTDEVICE8  pad;
        std::string error;
        DIJOYSTATE2 state;
        short id_pad;
        short id_count;
        short id_exclu;
        std::string nom_pad;

    public:
        Gamepad();
        ~Gamepad();
        BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context);
        std::string getLastError();
        void searchDevice();
        short open(std::string nom, short n=0);
        void close();
        std::string getName();
        bool synch();
        int getX();
        int getY();
        int getZ();
        BYTE* getButtons();

};

BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context);

#endif
