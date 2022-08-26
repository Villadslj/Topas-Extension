// Particle Generator for TripParticleGenerator
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//

#include "TripParticleGenerator.hh"
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


TripParticleGenerator::TripParticleGenerator(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
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
    file.open(ParticleSourceFile); 
	ResolveParameters();
    pM->SetNeedsSteppingAction();
	pM->SetNeedsTrackingAction();

}


TripParticleGenerator::~TripParticleGenerator()
{
}


void TripParticleGenerator::ResolveParameters() {
	TsVGenerator::ResolveParameters();
}


void TripParticleGenerator::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;
	
file.clear();
file.seekg(0, file.beg);    
TsPrimaryParticle p;
int max = 8379;
// int max = 9450;
int min = 2;

// Set the TsPrimaryParticle class values

auto lineNumberSought = round(G4UniformRand()*max);
   while (lineNumberSought < min || lineNumberSought > max )
   {
       lineNumberSought = round(G4UniformRand()*max);
   }
   

vector<double> row;
string line, csvItem;
int lineNumber = 0;
if (file.is_open()) {
    while (getline(file,line)) {
        lineNumber++;
        if(lineNumber == lineNumberSought) {
            istringstream myline(line);
            while(getline(myline, csvItem, ' ')) {
                row.push_back(stod(csvItem));
            }
        }
    }
}

// Lav Double gauss
// G4RandGauss
std::normal_distribution<double> distributionX(row[1],row[3]);
std::normal_distribution<double> distributionY(row[2],row[3]);
    p.posX = distributionX(generator) *cm;
    p.posY = distributionY(generator) *cm;
    p.posZ = BeamPositionZ;
    p.dCos1 = 0.0;
    p.dCos2 = 0.0;
    p.dCos3 = -1.0; // beam is in negative z direction
    p.kEnergy =  row[0] *GeV;
    p.isNewHistory = 0;
    p.weight = row[4];
    SetParticleType(p);
    GenerateOnePrimary(anEvent, p);

AddPrimariesToEvent(anEvent);


}