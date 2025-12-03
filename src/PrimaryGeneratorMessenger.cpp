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

#include "PrimaryGeneratorMessenger.h"
#include "PrimaryGeneratorAction.h"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction *gen)
    : generator(gen) {
    genDir = new G4UIdirectory("/generator/");
    genDir->SetGuidance("Controls for the primary particle generator");

    spectrumFileCmd = new G4UIcmdWithAString("/generator/setSpectrumFile", this);
    spectrumFileCmd->SetGuidance("Set spectrum filename (relative to spectra/ or absolute)");
    spectrumFileCmd->SetParameterName("filename", true);

    // photonFlux is unitless in our reporting (photons/s/mm2) -> use a plain double cmd
    photonFluxCmd = new G4UIcmdWithADouble("/generator/setPhotonFlux", this);
    photonFluxCmd->SetGuidance("Set photon flux (photons/s/mm2)");
    photonFluxCmd->SetParameterName("flux", false);

    monoCmd = new G4UIcmdWithABool("/generator/setMonochromatic", this);
    monoCmd->SetGuidance("Enable/disable monochromatic emission");
    monoCmd->SetParameterName("mono", false);

    monoEnergyCmd = new G4UIcmdWithADoubleAndUnit("/generator/setMonoEnergy", this);
    monoEnergyCmd->SetGuidance("Set monochromatic energy (e.g. 15.2 keV)");
    monoEnergyCmd->SetParameterName("energy", false);
    monoEnergyCmd->SetDefaultUnit("keV");
}

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {
    delete spectrumFileCmd;
    delete photonFluxCmd;
    delete monoCmd;
    delete monoEnergyCmd;
    delete genDir;
}

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {
    if (command == spectrumFileCmd) {
        generator->SetSpectrumFilename(std::string(newValue));
    } else if (command == photonFluxCmd) {
        const G4double val = G4UIcmdWithADouble::GetNewDoubleValue(newValue);
        PrimaryGeneratorAction::SetPhotonFlux(val);
    } else if (command == monoCmd) {
        const G4bool b = G4UIcmdWithABool::GetNewBoolValue(newValue);
        generator->SetMonochromatic(b);
    } else if (command == monoEnergyCmd) {
        const G4double e = G4UIcmdWithADoubleAndUnit::GetNewDoubleValue(newValue);
        generator->SetMonoEnergy(e);
    }
}
