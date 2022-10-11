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


    std::ifstream  data(ParticleSourceFile);
    if (!data.good()){
        G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "ParticleSource File does not exist" << G4endl;
        exit(1);

    }
    string line;
    while(getline(data, line))
    {   

        auto it = std::unique(line.begin(), line.end(),
            [](char const &lhs, char const &rhs) {
                return (lhs == rhs) && (lhs == ' ');
            });
        line.erase(it, line.end());
        replace(line.begin(), line.end(), ' ', ',');
        stringstream lineStream(line);
        string cell;
        vector<string> parsedRow;
        while(getline(lineStream, cell, ','))
        {   
            parsedRow.push_back(cell);
        }
        parsedCsv.push_back(parsedRow);
    }
   


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
	 
TsPrimaryParticle p;
int max = parsedCsv.size()-1;
int min = 1;

// Set the TsPrimaryParticle class values

int lineNumberSought = round(G4UniformRand()*max);
while (lineNumberSought < min || lineNumberSought > max )
{
    lineNumberSought = round(G4UniformRand()*max);
}


double Energy = atof(parsedCsv[lineNumberSought][0].c_str());
double X = atof(parsedCsv[lineNumberSought][1].c_str());
double Y = atof(parsedCsv[lineNumberSought][2].c_str());
double FWHM = atof(parsedCsv[lineNumberSought][3].c_str());
double W = atof(parsedCsv[lineNumberSought][4].c_str());

// Lav Double gauss
// G4RandGauss
std::normal_distribution<double> distributionX(X, FWHM/2.335); // Convert to sigma
std::normal_distribution<double> distributionY(Y, FWHM/2.335);
p.posX = distributionX(generator) *cm;
p.posY = distributionY(generator) *cm;
p.posZ = BeamPositionZ;
p.dCos1 = 0.0;
p.dCos2 = 0.0;
p.dCos3 = -1.0; // beam is in negative z direction
p.kEnergy =  Energy *GeV;
p.isNewHistory = 0;
p.weight = W;


SetParticleType(p);
GenerateOnePrimary(anEvent, p);

AddPrimariesToEvent(anEvent);


}

bool TripParticleGenerator::isBothSpace(char const &lhs, char const &rhs) {
    return lhs == rhs && iswspace(lhs);
}