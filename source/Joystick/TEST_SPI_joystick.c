#include "header/Joystick/SPI_joystick.h"

void spiJoystickInit(void)
{
    printf("Joystick inicializado!\n");
}
joystickID getSpiJoystickID(void)
{
    printf("Nuevo Joystick ID = Tu mama\n");
    return 1;
}
const SPI_Joystick_Events getSpiJoystickEvent(joystickID id)
{
    printf("Ultimo evento = Nada\n");
    return NO_JOYSTICK_EVENT;
}
void disableSpiJoystick(joystickID id)
{
    printf("Joystick deshabilitado :(\n");
}
void enableSpiJoystick(joystickID id)
{
    printf("Joystick habilitado!\n");
}
bool isSpiJoystickEnabled(joystickID id)
{
    printf("Is Joystick enabled? No idea :/\n");
    return true;
}