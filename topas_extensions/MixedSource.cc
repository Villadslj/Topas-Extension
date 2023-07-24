// Particle Generator for MixedSource
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//

#include "MixedSource.hh"
#include "TsParameterManager.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ChargedGeantino.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <G4ParticleGun.hh>
#include <G4Proton.hh>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <random>


using namespace std;


MixedSource::MixedSource(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
TsVGenerator(pM, gM, pgM, sourceName)
{
    if (fPm->ParameterExists(GetFullParmName("Particles"))){
		auto G4Secondaries = fPm->GetStringVector(GetFullParmName("Particles"));
		auto slength = fPm->GetVectorLength(GetFullParmName("Particles"));
		for (int i=0;i<slength;i++){
			DSecondaries.push_back(G4Secondaries[i]);
		}

	}
    if (!fPm->ParameterExists(GetFullParmName("BeamPositionZ"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "BeamPositionZ parameter is missing " << G4endl;
		exit(1);
	} else {
		BeamPositionZ = fPm->GetDoubleParameter(GetFullParmName("BeamPositionZ"), "Length");
	}

    if (!fPm->ParameterExists(GetFullParmName("MixingFraction"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "MixingFraction parameter is missing " << G4endl;
		exit(1);
	} else {
		auto MixingFraction = fPm->GetUnitlessVector(GetFullParmName("MixingFraction"));
		auto slength = fPm->GetVectorLength(GetFullParmName("MixingFraction"));
		for (int i=0;i<slength;i++){
			DMixingFraction.push_back(MixingFraction[i]);
		}
	}

    if (!fPm->ParameterExists(GetFullParmName("MonoEnergies"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "MonoEnergies parameter is missing " << G4endl;
		exit(1);
	} else {
		auto MonoEnergies = fPm->GetDoubleVector(GetFullParmName("MonoEnergies"), "Energy");
		auto slength = fPm->GetVectorLength(GetFullParmName("MonoEnergies"));
		for (int i=0;i<slength;i++){
			DMonoEnergies.push_back(MonoEnergies[i]);
		}
	}
    if (!fPm->ParameterExists(GetFullParmName("SpotSize"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "SpotSize parameter is missing " << G4endl;
		exit(1);
	} else {
		SpotSize = fPm->GetDoubleParameter(GetFullParmName("SpotSize"), "Length");
	}



    ResolveParameters();
    pM->SetNeedsSteppingAction();
	pM->SetNeedsTrackingAction();

}


MixedSource::~MixedSource()
{
}


void MixedSource::ResolveParameters() {
	TsVGenerator::ResolveParameters();
}


void MixedSource::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;
	 
TsPrimaryParticle p;

// Set the TsPrimaryParticle class values

int RandN = round(G4UniformRand());

// Select the particle based on MixingFraction weights
double cumulativeWeight = 0.0;
int selectedParticleIndex = 0;

// Calculate cumulative weights
for (int i = 0; i < DMixingFraction.size(); i++) {
    cumulativeWeight += DMixingFraction[i];
    if (RandN <= cumulativeWeight) {
        selectedParticleIndex = i;
        break;
    }
}

// Set the particle type based on selectedParticleIndex
p.particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle(DSecondaries[selectedParticleIndex]);

// Set the energy based on MonoEnergies
if (selectedParticleIndex < DMonoEnergies.size()) {
    p.kEnergy = DMonoEnergies[selectedParticleIndex] * MeV;
} else {
    // Handle the case when MonoEnergies vector is shorter than DSecondaries vector
    G4cerr << "MonoEnergies vector is shorter than DSecondaries vector." << G4endl;
    // You can decide what to do in this case, such as setting a default energy.
    // Here, I'm setting it to 0 GeV.
    p.kEnergy = 0.0 * GeV;
}


// Lav Double gauss
// G4RandGauss
std::normal_distribution<double> distributionX(0, SpotSize/2.335); // Convert to sigma
std::normal_distribution<double> distributionY(0, SpotSize/2.335);
p.posX = distributionX(generator) *cm;
p.posY = distributionY(generator) *cm;
p.posZ = BeamPositionZ;
p.dCos1 = 0.0;
p.dCos2 = 0.0;
p.dCos3 = -1.0; // beam is in negative z direction
p.isNewHistory = 0;
p.weight = 1.;


GenerateOnePrimary(anEvent, p);
AddPrimariesToEvent(anEvent);


}

bool MixedSource::isBothSpace(char const &lhs, char const &rhs) {
    return lhs == rhs && iswspace(lhs);
}