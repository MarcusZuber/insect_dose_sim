/*
 * Geant4 based dose simulation for insects
 * Copyright (C) 2025
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef DOSE_SIM_BEETLES_PARAMETERS_H
#define DOSE_SIM_BEETLES_PARAMETERS_H
#include <G4Types.hh>
#include <CLHEP/Units/SystemOfUnits.h>

inline G4double beamSize = 10 * CLHEP::mm;
inline G4double beamArea = beamSize * beamSize; // mm2


#endif //DOSE_SIM_BEETLES_PARAMETERS_H
