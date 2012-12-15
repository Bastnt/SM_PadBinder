#include <iostream>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

using namespace std;

    LPDIRECTINPUT8 di;
    LPDIRECTINPUTDEVICE8  pad;

    BOOL CALLBACK CreateDeviceCallback(
		LPCDIDEVICEINSTANCE instance,
		LPVOID reference)
        {
            HRESULT result;

            // Cr�ation du p�riph�rique
            result = di_object->CreateDevice( instance->guidInstance, &xpad, NULL);

            // Gestion des erreurs
            if(FAILED( result ))
                return DIENUM_CONTINUE;

            // En cas de r�ussite ... on arr�te l'enum�ration
            return DIENUM_STOP;
        }

int main()
{
    // ==== Cr�ation de l'instance DI
    HRESULT result;
    HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);


    result = DirectInput8Create(hinst,
                                DIRECTINPUT_VERSION,
                                IID_IDirectInput8,
                                (void **)&di_object,
                                NULL);

    if (FAILED(result))
    {
        cout << "Error in DI Instance summoning." << endl;
        return 1;
    }

    // ==== Connexion avec le premier PAD trouv�


    result  = di_object->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                    &CreateDeviceCallback,
                                    NULL,
                                    DIEDFL_ATTACHEDONLY);

    // ==== Communication avec le PAD
    result=xpad->SetDataFormat(&c_dfDIJoystick2);
    if(FAILED(result))
    {
        cout << "Probl�me config du format";
        return 3;
    }

    // ==== R�cup�ration des �tats du PAD
    DIJOYSTATE2 state;

    if(FAILED(result))
    {
        cout << "Impossible de r�cup�rer l'etat du p�riph�rique.";
        return 5;
    }
        xpad->Acquire();

        DIDEVICEINSTANCE device;
        ZeroMemory(&device, sizeof(device));
        device.dwSize = sizeof(device);
        xpad->GetDeviceInfo(&device);


        while(1)
        {
            result = xpad->GetDeviceState(
            sizeof(state),
            (LPVOID)&state);
            if(FAILED(result))
            {
                cout << "Impossible de r�cup�rer l'etat du p�riph�rique.";
                return 5;
            }
            if(state.rgbButtons[0])
                break;
            cout << "X : " << state.lX << endl;
            cout << "Y : " << state.lY << endl;
            cout << "DEVICE : " << device.tszProductName;
            Sleep(100);
            system("CLS");
        }

        xpad->Unacquire();
        xpad->Release();
        di_object->Release();

    cout << "Programme fini.";

    return 0;
}
