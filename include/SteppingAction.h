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

#ifndef SteppingAction_h
#define SteppingAction_h

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include <map>
#include <string>

class SteppingAction final : public G4UserSteppingAction {
public:
    SteppingAction();

    ~SteppingAction() override;

    /**
     * Stepping Action to accumulate energy deposition per volume
     *
     * the *doseMap* holds the dose for each volume by name.
     *
     * @param step
     */
    void UserSteppingAction(const G4Step *step) override;

    /**
     * Getter for dose map
     * @return map of volume name to accumulated dose
     */
    static std::map<std::string, G4double> &getDoseMap();


    /**
     * Getter for volume map
     * @return map of volume name to volume
     */
    static std::map<std::string, G4double> &getVolumeMap();

    /**
     * Stores the (qubic) volume for a given (scoring) volume name
     * @param name Name of the volume
     * @param volume volume (ideally with units)
     */
    static void setVolume(const std::string &name, const G4double volume) { volumeMap[name] = volume; }

private:
    /**
     * Map of volume name to accumulated dose (in MeV)
     */
    static std::map<std::string, G4double> doseMap;

    /**
     * Map of volume name to volume (in mm3)
     */
    static std::map<std::string, G4double> volumeMap;
};

#endif
