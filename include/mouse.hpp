#pragma once

#include <glm/glm.hpp>

void MouseInit();
void MouseQuit();
void MouseMove(const glm::ivec2 &coords);
void MouseLeftPress();
void MouseLeftRelease();
bool MouseValid();
