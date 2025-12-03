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

#ifndef PrimaryGeneratorMessenger_h
#define PrimaryGeneratorMessenger_h

#include "G4UImessenger.hh"
#include "G4String.hh"

class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithABool;
class G4UIcmdWithADouble;
class PrimaryGeneratorAction;

class PrimaryGeneratorMessenger final : public G4UImessenger {
public:
    explicit PrimaryGeneratorMessenger(PrimaryGeneratorAction *gen);

    ~PrimaryGeneratorMessenger() override;

    void SetNewValue(G4UIcommand *command, G4String newValue) override;

private:
    PrimaryGeneratorAction *generator;
    G4UIdirectory *genDir{nullptr};
    G4UIcmdWithAString *spectrumFileCmd{nullptr};
    G4UIcmdWithADouble *photonFluxCmd{nullptr};
    G4UIcmdWithABool *monoCmd{nullptr};
    G4UIcmdWithADoubleAndUnit *monoEnergyCmd{nullptr};
};

#endif
