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

#ifndef RunAction_h
#define RunAction_h

#include "G4UserRunAction.hh"
#include "globals.hh"
#include <string>

class RunMessenger; // forward

class RunAction final : public G4UserRunAction {
public:
    RunAction();

    ~RunAction() override;

    void BeginOfRunAction(const G4Run *run) override;

    void EndOfRunAction(const G4Run *run) override;

    // Allow macro to configure output filename prefix
    void SetOutputFilePrefix(const std::string &prefix);

    [[nodiscard]] const std::string &GetOutputFilePrefix() const;

private:
    // configurable output prefix (default 'dose_results_')
    std::string outputPrefix{"dose_results_"};

    // messenger to receive macro commands
    RunMessenger *messenger{nullptr};
};

#endif
