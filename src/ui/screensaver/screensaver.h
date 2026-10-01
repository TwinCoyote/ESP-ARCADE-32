#pragma once

// Protector de pantalla: animaciones que se turnan cuando nadie toca los botones
// (mascota virtual, ojos flotantes y un video musical psicodelico).
// SystemManager::animation() decide cuando entra y cuando sale; aqui solo se dibuja.
namespace screensaver
{
    // Arranca: disuelve lo que habia en pantalla y sigue con la siguiente escena de la lista
    void begin();
    // Avanza y dibuja un cuadro. Va en cada vuelta del loop mientras el protector este activo
    void update();
}
