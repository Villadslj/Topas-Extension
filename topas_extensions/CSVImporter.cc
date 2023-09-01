// Particle Generator for CSVImporter
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//

#include "CSVImporter.hh"
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
#include "G4RotationMatrix.hh"
#include "G4ParticleTable.hh"


using namespace std;


CSVImporter::CSVImporter(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
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


    // Open the CSV file
    std::ifstream file(ParticleSourceFile);
    if (!file.is_open()) {
        G4cerr << "Error: Could not open file " << ParticleSourceFile << G4endl;
        exit(1);
    }

    // Read each line in the CSV file
    std::string line;
    while (getline(file, line)) {
        // Remove the double quotes and square brackets at the start and end of the line
        line = line.substr(2, line.size() - 2);
        std::stringstream ss(line);

        // Read each value from the stringstream and store in respective vectors
        double value;
        char comma;
        ss >> value;
        col1.push_back(value);
        ss >> comma;
        ss >> value;
        col2.push_back(value);
        ss >> comma;
        ss >> value;
        col3.push_back(value);
        ss >> comma;
        ss >> value;
        col4.push_back(value);
        ss >> comma;
        ss >> value;
        col5.push_back(value);
        ss >> comma;
        ss >> value;
        col6.push_back(value);
        ss >> comma;
        ss >> value;
        col7.push_back(value);
        ss >> comma;
        ss >> value;
        col8.push_back(value);
    }
   


    ResolveParameters();


}


CSVImporter::~CSVImporter()
{
}


void CSVImporter::ResolveParameters() {
	TsVGenerator::ResolveParameters();
}


void CSVImporter::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;
	 
TsPrimaryParticle p;
int max = col8.size()-1;
int min = 1;

// Set the TsPrimaryParticle class values

int lineNumberSought = round(G4UniformRand()*max);
while (lineNumberSought < min || lineNumberSought > max )
{
    lineNumberSought = round(G4UniformRand()*max);
}


// Retrieve the particle information from the parsed CSV data
double Energy = col8[lineNumberSought];
double X = col3[lineNumberSought];
double Z = col4[lineNumberSought];
double FWHM = 0.0; // You need to specify the column index for FWHM in your CSV file
double W = 1.0; // You need to specify the column index for W in your CSV file
// G4cout << X << "  " << Z << G4endl;
// Coordinate Transformation
// G4RotationMatrix rotationMatrix;
// rotationMatrix.rotateX(90.0 * deg); // Rotate around the X-axis by 90 degrees

// G4ThreeVector initialPosition(X, Y, Z);
// G4ThreeVector rotatedPosition = rotationMatrix * initialPosition;

// Lav Double gauss
// G4RandGauss
p.posX = X *mm;
p.posY = Z *mm;    
p.posZ = BeamPositionZ;
p.dCos1 = 0.0;
p.dCos2 = 0.0;
p.dCos3 = -1.0; // beam is in negative z direction
p.kEnergy =  Energy *MeV;
p.isNewHistory = 0;
p.weight = W;

G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
G4ParticleDefinition* particle = particleTable->FindParticle(col1[lineNumberSought]);
p.particleDefinition = particle;

// SetParticleType(p);
GenerateOnePrimary(anEvent, p);

AddPrimariesToEvent(anEvent);


}

bool CSVImporter::isBothSpace(char const &lhs, char const &rhs) {
    return lhs == rhs && iswspace(lhs);
}