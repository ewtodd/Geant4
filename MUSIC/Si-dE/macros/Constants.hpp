#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <TROOT.h>
#include <TString.h>
#include <vector>

namespace Constants {

// Energy deposited in the active Si per event, in MeV (event.cc).
const TString ENERGY_TREE = "Energy";
const TString ENERGY_BRANCH = "fEdep";

// The five runs peak between 40 and 77 MeV with sigma of about 0.2 MeV; under
// 1e-4 of the entries lie in a low tail outside this window.
const Int_t EDEP_BIN_WIDTH_KEV = 50;
const Int_t EDEP_XMIN_MEV = 35, EDEP_XMAX_MEV = 80;
const Int_t EDEP_NBINS =
    (EDEP_XMAX_MEV - EDEP_XMIN_MEV) * 1000 / EDEP_BIN_WIDTH_KEV;

const TString ENTER_TI_71MEV = "EnterTi-71Point35MeVBeam";
const TString ENTER_TI_92MEV = "EnterTi-91Point87MeVBeam";
const TString ENTER_EXIT_TI_71MEV = "EnterTi-ExitTi-71Point35MeVBeam";
const TString ENTER_EXIT_TI_92MEV = "EnterTi-ExitTi-92Point0MeVBeam";
const TString ENTER_EXIT_TI_DEAD_LAYERS_92MEV =
    "EnterTi-ExitTi-100nmFront-200nmRear-92Point0MeVBeam";
const std::vector<TString> ALL_CONFIGURATIONS = {
    ENTER_TI_71MEV, ENTER_TI_92MEV, ENTER_EXIT_TI_71MEV, ENTER_EXIT_TI_92MEV,
    ENTER_EXIT_TI_DEAD_LAYERS_92MEV};
const std::vector<TString> CONFIGURATION_LABELS = {
    "Entrance Ti, 71.35 MeV", "Entrance Ti, 91.87 MeV",
    "Entrance + exit Ti, 71.35 MeV", "Entrance + exit Ti, 92.0 MeV",
    "Entrance + exit Ti, Si dead layers, 92.0 MeV"};

} // namespace Constants
#endif
