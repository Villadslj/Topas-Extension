// Particle Generator for PLDParticleSource
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2023                                       *
// * For including source files from ROOT                             *
// *                                                                  *
// ********************************************************************
//

#include "PLDParticleSource.hh"
#include "TsParameterManager.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <vector>




using namespace std;


PLDParticleSource::PLDParticleSource(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName)
{
    if (!fPm->ParameterExists(GetFullParmName("ParticleSourceFile"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "ParticleSourceFile parameter is missing " << G4endl;
		exit(1);
	} else {
		ParticleSourceFile = fPm->GetStringParameter(GetFullParmName("ParticleSourceFile"));
	}

    if (!fPm->ParameterExists(GetFullParmName("BeamPositionZ"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "BeamPositionZ parameter is missing " << G4endl;
		exit(1);
	} else {
		BeamPositionZ = fPm->GetDoubleParameter(GetFullParmName("BeamPositionZ"), "Length");
	}

    if (!fPm->ParameterExists(GetFullParmName("EnergySpread"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "BeamPositionZ parameter is missing " << G4endl;
		exit(1);
	} else {
		EnergySpread = fPm->GetUnitlessParameter(GetFullParmName("EnergySpread"));
        // G4cout << EnergySpread << G4endl;
	}

    if (!fPm->ParameterExists(GetFullParmName("SpotFWHM"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "ParticleSourceFile parameter is missing " << G4endl;
		exit(1);
	} else {
		SpotFWHM = fPm->GetDoubleParameter(GetFullParmName("SpotFWHM"), "Length");
	}

    LoadPLDFile(ParticleSourceFile.c_str());


    ResolveParameters();
    pM->SetNeedsSteppingAction();
	pM->SetNeedsTrackingAction();

}


PLDParticleSource::~PLDParticleSource()
{
}


void PLDParticleSource::ResolveParameters() {
	TsVGenerator::ResolveParameters();
}


void PLDParticleSource::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;


    TsPrimaryParticle p;
    // Randomly select a layer
    int selectedLayerIndex = G4int(G4UniformRand() * layers.size());
    const Layer& selectedLayer = layers[selectedLayerIndex];

    // Randomly select an element within the selected layer
    int selectedElementIndex = G4int(G4UniformRand() * selectedLayer.elements.size());
    const SpotElement& selectedElement = selectedLayer.elements[selectedElementIndex];

    // Set the particle properties using the selected element data
    G4ThreeVector position(selectedElement.positionX, selectedElement.positionY, 0);
    
    // Assuming the energy in the file is in MeV
    G4double energy = selectedLayer.energy * MeV;
    
    std::normal_distribution<double> distributionX(selectedElement.positionX, SpotFWHM/2.335); // Convert to sigma
    std::normal_distribution<double> distributionY(selectedElement.positionY, SpotFWHM/2.335);
    std::normal_distribution<double> distributionE(energy, (energy*EnergySpread/100)/2.335);
	p.posX = distributionX(generator) * mm;
	p.posY = distributionY(generator) * mm; 
	p.posZ = BeamPositionZ;
    // G4cout << "test" << G4endl;
    // G4cout << p.posX <<' '<< p.posY << G4endl; 

	// This is for sure cheating. So if you are not Villads, don't do this
	
	p.dCos1 = 0;
	p.dCos2 = 0;
	p.dCos3 = -1; 
	p.kEnergy =  distributionE(generator) * MeV;
    p.weight = selectedElement.meterSetWeight/cumulativeMeterSetWeight;
	p.isNewHistory = 0;

	SetParticleType(p);
	GenerateOnePrimary(anEvent, p);

	AddPrimariesToEvent(anEvent);


}

bool PLDParticleSource::isBothSpace(char const &lhs, char const &rhs) {
    return lhs == rhs && iswspace(lhs);
}

void PLDParticleSource::LoadPLDFile(const char* ParticleSourceFile)
{
    std::ifstream file(ParticleSourceFile);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << ParticleSourceFile << std::endl;
        return;
    }

    std::string line;

    // Parse the header line
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }
        totalMU = std::stod(tokens[7]);
        cumulativeMeterSetWeight = std::stod(tokens[8]);
        numberOfLayers = std::stoi(tokens[9]);
    }

    // Parse each layer and element
    Layer currentLayer;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }

        std::string firstWord = tokens[0];

        if (firstWord == "Layer") {
            // Save the current layer if it contains data
            if (!currentLayer.elements.empty()) {
                layers.push_back(currentLayer);
                currentLayer.elements.clear();
            }

            currentLayer.energy = std::stod(tokens[2]);
            currentLayer.SumMeterSetWeight = std::stod(tokens[3]);
            currentLayer.numElements = std::stoi(tokens[4]);

        } else if (firstWord == "Element") {
            SpotElement element;
            element.positionX = std::stod(tokens[1]);
            element.positionY = std::stod(tokens[2]);
            element.meterSetWeight = std::stod(tokens[3]);
            element.meterSetRate = std::stod(tokens[4]);

            currentLayer.elements.push_back(element);
        }
    }
    // Save the last layer
    if (!currentLayer.elements.empty()) {
        layers.push_back(currentLayer);
    }
}

