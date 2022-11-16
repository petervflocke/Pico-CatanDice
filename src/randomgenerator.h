/* cSpell:disable */
#pragma once
#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include "Arduino.h"

#define analog0 26
#define analog1 27

void seedrnd(unsigned long t, uint32_t &randomVal);

#endif