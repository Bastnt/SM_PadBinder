#include "gamepad.h"

std::string Gamepad::getLastError()
{
    return error;
}

// === Constructeur : Invocation de DI
Gamepad::Gamepad()
{
    error="";
    id_count=0;
    id_pad=0;
    di=NULL;
    pad=NULL;
    id_exclu=0;
    nom_pad="";

    HRESULT result;
    HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
    result = DirectInput8Create(hinst,
                                DIRECTINPUT_VERSION,
                                IID_IDirectInput8,
                                (void **)&di,
                                NULL);
    if (FAILED(result))
    {
        std::cout << "Erreur dans l'invocation de Direct Input.";
    }
}

// === Déstructeur : Désinvocation de DI
Gamepad::~Gamepad()
{
    di->Release();
}

// === Connexion au PAD
short Gamepad::open(std::string nom, short n)
{
    id_exclu=n;
    id_count=0;
    nom_pad=nom;
    HRESULT result;
    result  = di->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                    ::enumCallback,
                                    (LPVOID)this,
                                    DIEDFL_ATTACHEDONLY);
    if(FAILED(result))
    {
        error="Impossible de se connecter aux peripheriques USB.";
        return 0;
    }

    if (!pad)
    {
        error="Auncun peripherique trouve";
        return 0;
    }

    result=pad->SetDataFormat(&c_dfDIJoystick2);
    if(FAILED(result))
    {
        error="Impossible de definir le format.";
        return 0;
    }

    pad->Acquire();

    return id_pad;
}

void Gamepad::close()
{
    if(pad)
    {
        pad->Unacquire();
        pad->Release();
    }
}

std::string Gamepad::getName()
{
    DIDEVICEINSTANCE device;
    ZeroMemory(&device, sizeof(device));
    device.dwSize = sizeof(device);
    pad->GetDeviceInfo(&device);
    return device.tszProductName;
}

bool Gamepad::synch()
{
    if(SUCCEEDED(pad->GetDeviceState(sizeof(state), (LPVOID)&state)))
        return true;
    else
        return false;
}

int Gamepad::getX()
{
    return state.lX;
}
int Gamepad::getY()
{
    return state.lY;
}
int Gamepad::getZ()
{
    return state.lZ;
}

BYTE* Gamepad::getButtons()
{
    return state.rgbButtons;
}


// Fonction de valdation du périphérique trouvé

BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
    if (context != NULL) {
        return ((Gamepad *)context)->enumCallback(instance, context);
    } else {
        return DIENUM_STOP;
    }
}

BOOL CALLBACK Gamepad::enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
    id_count++;
    if(id_exclu != 0 && id_count == id_exclu)
    {
        return DIENUM_CONTINUE;
    }



    if (SUCCEEDED(di->CreateDevice(instance->guidInstance, &pad, NULL)))
        if((getName()).substr(0,nom_pad.length()) == nom_pad)
        {
            id_pad=id_count;
            return DIENUM_STOP;
        }

    return DIENUM_CONTINUE;
}
