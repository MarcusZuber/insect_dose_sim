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

#include "PhysicsList.h"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4SystemOfUnits.hh"
#include "G4EmParameters.hh"

PhysicsList::PhysicsList() {
    SetVerboseLevel(1);

    // Penelope physics for accurate low-energy photon interactions
    RegisterPhysics(new G4EmPenelopePhysics());

    // Decay physics
    RegisterPhysics(new G4DecayPhysics());

    // Configure EM parameters for maximum precision
    G4EmParameters *emParams = G4EmParameters::Instance();
    emParams->SetStepFunction(0.1, 0.01 * mm); // More restrictive step function
    emParams->SetStepFunctionMuHad(0.05, 0.005 * mm); // Even finer for muons/hadrons
    G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(250 * eV, 1 * GeV);
}

PhysicsList::~PhysicsList()
= default;

void PhysicsList::SetCuts() {
    // Set very small production cuts for high precision in small volumes
    // Smaller cuts = more accurate tracking at the cost of computation time
    SetCutValue(0.01 * mm, "gamma"); // 10 micrometers
    SetCutValue(0.01 * mm, "e-"); // 10 micrometers
    SetCutValue(0.01 * mm, "e+"); // 10 micrometers
    SetCutValue(0.01 * mm, "proton"); // 10 micrometers

    if (verboseLevel > 0) {
        DumpCutValuesTable();
    }
}
