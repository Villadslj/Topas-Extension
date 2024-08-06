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
#include <fstream>
// #include <iostream>


NuclearReactionScorer::NuclearReactionScorer(
    TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM,
    TsScoringManager* scM, TsExtensionManager* eM, G4String scorerName,
    G4String quantity, G4String outFileName, G4bool isSubScorer)
    : TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
    G4cout << outFileName << G4endl;
    csvFileName=outFileName;
    std::ofstream outputFile(outFileName + ".csv", std::ios::trunc);
    if (outputFile.is_open()) {
        // File exists, truncate it (delete contents)
        outputFile.close();
        // std::cout << "Cleared contents of file: " << outFileName + ".csv" << std::endl;
    } else {
        std::cerr << "Error opening file: " << outFileName + ".csv" << std::endl;
    }

    if (fPm->ParameterExists(GetFullParmName("MyFileName"))) {
		G4String MyFileName = fPm->GetStringParameter(GetFullParmName("MyFileName"));
        G4cout << MyFileName << G4endl;
	
	} 


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

    parentE = startPoint->GetKineticEnergy();

    // Secondary particles
    auto secondary = aStep->GetSecondaryInCurrentStep();
    size_t size_secondary = secondary->size();

    if (size_secondary == 0) {
        return false;
    }

    std::vector<G4String> vec_name;

    for (auto t : *secondary) {
        vec_name.push_back(t->GetDefinition()->GetParticleName());
    }

    std::sort(vec_name.begin(), vec_name.end());
    std::unordered_map<std::string, int> particleCount; // Map to store counts of each particle type
    std::vector<std::string> vec_name2 = {"C13", "neut", "pi+", "pi-", "pi0", "pi0", "pi0", "prot"};

    // Count occurrences of each string in vec_name
    for (const auto& name : vec_name) {
        particleCount[name]++;
    }
    // Construct the secondaries string
    for (const auto& pair : particleCount) {
        if (!secondaries.empty()) {
            secondaries += " + ";
        }
        if (pair.second > 1) {
            secondaries += std::to_string(pair.second) + " ";
        }
        secondaries += pair.first;
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

    std::stringstream data;
    data << parentE << "," << processname << "," << projectile << "," << targetName << "," << secondaries << "," << pAlive << std::endl;

    // Write data to CSV file
    WriteToCSV(csvFileName, data.str());
    
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


void NuclearReactionScorer::WriteToCSV(const std::string& filename, const std::string& data) {
    std::ofstream outputFile;
    outputFile.open(filename + ".csv", std::ios_base::app); // Open in append mode

    if (!outputFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    outputFile << data;
    outputFile.close();
}