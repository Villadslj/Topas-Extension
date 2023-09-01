// Scorer for NuclearReaction
// Created by Villads J. 2022
// For scoring nuclear reaction

#include "NuclearReactionScorer.hh"
#include "TsTrackInformation.hh"
#include "G4PSDirectionFlag.hh"
#include "G4VProcess.hh"
#include "G4HadronicProcess.hh"
#include "G4ParticleTypes.hh"
#include "G4RunManager.hh"
#include "G4SteppingManager.hh"

NuclearReactionScorer::NuclearReactionScorer(
    TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
    TsScoringManager* scM, TsExtensionManager* eM, G4String scorerName,
    G4String quantity, G4String outFileName, G4bool isSubScorer)
    : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
    // Register columns for data storage in the ntuple
    fNtuple->RegisterColumnD(&parentE, "parentE", "MeV");
    fNtuple->RegisterColumnS(&processname, "processname");
    fNtuple->RegisterColumnS(&projectile, "projectile");
    fNtuple->RegisterColumnS(&targetName, "targetName");
    fNtuple->RegisterColumnS(&secondaries, "secondaries");
    fNtuple->RegisterColumnS(&pAlive, "pAlive");

    // Initialize some parameters from the input parameters
    DTarget = fPm->ParameterExists(GetFullParmName("Target"))
                  ? fPm->GetStringParameter(GetFullParmName("Target"))
                  : "";
    DProjectile = fPm->ParameterExists(GetFullParmName("Projectile"))
                      ? fPm->GetStringParameter(GetFullParmName("Projectile"))
                      : "";

    // Initialize secondary particle lists
    InitializeSecondaryList(DSecondaries, "Secondaries", slength);
    InitializeSecondaryList(DSecondaries2, "Secondaries2", slength2);
}

NuclearReactionScorer::~NuclearReactionScorer() {}

G4bool NuclearReactionScorer::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    ResolveSolid(aStep);
    ClearParameters();
    
    if (!fIsActive) {
        fSkippedWhileInactive++;
        return false;
    }
    
    // Get information about the interaction
    auto startPoint = aStep->GetPreStepPoint();
    auto endPoint = aStep->GetPostStepPoint();
    const G4VProcess* process = endPoint->GetProcessDefinedStep();
    G4String nuclearChannel = process->GetProcessName() + " : ";
    processname = process->GetProcessName();
    
    auto hproc = const_cast<G4HadronicProcess*>(static_cast<const G4HadronicProcess *>(process));
    const G4Isotope* target = nullptr;

    if (hproc && process->GetProcessType() == 4) {
        target = hproc->GetTargetIsotope();
    }

    targetName = target ? target->GetName() : "XXXX";
    projectile = aStep->GetTrack()->GetDefinition()->GetParticleName();

    // Special case for "RadioactiveDecayBase"
    if (processname == "RadioactiveDecayBase") {
        targetName = "";
    }

    nuclearChannel += aStep->GetTrack()->GetDefinition()->GetParticleName() + " + " + targetName + " -->";
    parentE = startPoint->GetKineticEnergy();

    // Secondary particles
    auto secondary = aStep->GetSecondaryInCurrentStep();
    G4String test = "";
    size_t size_secondary = secondary->size();

    if (size_secondary == 0) {
        return false;
    }

    std::vector<G4String> vec_name;

    for (auto t : *secondary) {
        vec_name.push_back(t->GetDefinition()->GetParticleName());
    }

    std::sort(vec_name.begin(), vec_name.end());
    int first = 0;
    std::unordered_map<std::string, int> particleCount; // Map to store counts of each particle type

    for (auto name : vec_name) {
        if (name == "e-" || name == "gamma") {
            continue;
        }

        if (particleCount.find(name) == particleCount.end()) {
            particleCount[name] = 1; // Initialize count to 1 for the first occurrence
        } else {
            particleCount[name]++; // Increment the count for subsequent occurrences
        }

        if (first == 0) {
            nuclearChannel += name;
            test += name;
            secondaries += name.substr(0, 4);
            first += 1;
        } else {
            // Append the count (if greater than 1) in front of the particle name
            if (particleCount[name] > 1) {
                nuclearChannel += " + " + std::to_string(particleCount[name]) + " " + name;
                test += " + " + std::to_string(particleCount[name]) + " " + name;
                secondaries += " + " + std::to_string(particleCount[name]) + " " + name.substr(0, 4);
            } else {
                nuclearChannel += " + " + name;
                test += " + " + name;
                secondaries += " + " + name.substr(0, 4);
            }
        }
    }

    if (aStep->GetTrack()->GetTrackStatus() == fAlive) {
        nuclearChannel += " + " + aStep->GetTrack()->GetDefinition()->GetParticleName();
        G4String particleAlive = aStep->GetTrack()->GetDefinition()->GetParticleName();
        pAlive +=  particleAlive.substr(0, 4);
    }

    // Filter reactions based on criteria
    if (targetName != DTarget && DTarget != "") {
        return false;
    }
    
    if (projectile != DProjectile && DProjectile != "") {
        return false;
    }
    
    if (!DSecondaries.empty() || !DSecondaries2.empty()) {
        bool SecondariesTest = CheckSecondaries(DSecondaries, vec_name, slength);
        
        if (!SecondariesTest) {
            return false;
        }
    }
    
    FillEmptyParm();
    fNtuple->Fill();
    return true;
}

void NuclearReactionScorer::ClearParameters()
{
    projectile = "";
    processname = "";
    targetName = "";
    secondaries = "";
    pAlive = "";
    parentE = 0;
}

void NuclearReactionScorer::FillEmptyParm()
{
    if (projectile.empty()) {
        projectile = "empty";
    }
    
    if (processname.empty()) {
        processname = "empty";
    }
    
    if (targetName.empty()) {
        targetName = "empty";
    }
    
    if (secondaries.empty()) {
        secondaries = "empty";
    }
    
    if (pAlive.empty()) {
        pAlive = "empty";
    }
}

bool NuclearReactionScorer::CheckSecondaries(std::vector<G4String>& Test, std::vector<G4String>& real, int l)
{
    if (l == 0) {
        return false;
    }

    std::sort(Test.begin(), Test.end());
    std::sort(real.begin(), real.end());
    bool goodtest = true;

    for (int i = 0; i < l; i++) {
        int n_test = std::count(Test.begin(), Test.end(), Test[i]);
        int n_real = std::count(real.begin(), real.end(), Test[i]);

        if (!std::count(real.begin(), real.end(), Test[i]) || n_test > n_real) {
            goodtest = false;
        }
    }

    return goodtest;
}

void NuclearReactionScorer::InitializeSecondaryList(std::vector<G4String>& list, const G4String& parameterName, int& length)
{
    if (!fPm->ParameterExists(GetFullParmName(parameterName))) {
        list.clear();
        length = 0;
    }

    if (fPm->ParameterExists(GetFullParmName(parameterName))) {
        auto G4Secondaries = fPm->GetStringVector(GetFullParmName(parameterName));
        length = fPm->GetVectorLength(GetFullParmName(parameterName));
        
        for (int i = 0; i < length; i++) {
            list.push_back(G4Secondaries[i]);
        }
    }
}
