// Particle Generator for RootParticleSource
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//

#include "RootParticleSource.hh"
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
#include "TFile.h"
#include "TNtuple.h"
#include "TTree.h"
#include <vector>
#include "G4RotationMatrix.hh"

using namespace std;


RootParticleSource::RootParticleSource(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName) :
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

    if (!fPm->ParameterExists(GetFullParmName("NumberOfHistoriesInRun"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "NumberOfHistoriesInRun parameter is missing " << G4endl;
		exit(1);
	} else {
		NoOfHist = fPm->GetIntegerParameter(GetFullParmName("NumberOfHistoriesInRun"));
	}
    
    TFile f(ParticleSourceFile, "READ");
	TTree* n;
	n = (TTree*) f.Get("ntuple1");
	Long64_t PSpartID, PSparentID;
	Float_t PSposX, PSposY, PSposZ, PSpx, PSpy, PSpz, PSKE;

    n -> SetBranchAddress("PSpartID",&PSpartID);
	n -> SetBranchAddress("PSparentID", &PSparentID);
	//n -> SetBranchAddress("PSposX", &PSposX); //Not used for HIMAC PSFs
	n -> SetBranchAddress("PSposY", &PSposY);
	n -> SetBranchAddress("PSposZ", &PSposZ);
	n -> SetBranchAddress("PSpx", &PSpx);
	n -> SetBranchAddress("PSpy", &PSpy);
	n -> SetBranchAddress("PSpz", &PSpz);
	n -> SetBranchAddress("PSKE", &PSKE);
	numberEntries = n->GetEntries();
    G4int eventNumber;
    G4int eID;
    G4int evtsRead = 0;
    G4int entriesPerRead = NoOfHist;
    G4int randEntryNum = CLHEP::RandFlat::shoot(numberEntries-entriesPerRead); 
	
	for (int ri = randEntryNum; ri < (randEntryNum+entriesPerRead); ri++)//for random reading
    //	for (int ri = evtsRead; ri < (endRead); ri++)// reading from start of file
	{
		//G4int randEntryNum = CLHEP::RandFlat::shoot(numberEntries);
		//G4cout << "Entry number: " << randEntryNum << G4endl;
		//G4int j = i%count;
	
		n->GetEntry(ri);
		rPartIDv.push_back(PSpartID);
		rxPosv.push_back(PSparentID);
		ryPosv.push_back(PSposY);
		rzPosv.push_back(PSposZ);
		rpxv.push_back(PSpx);
		rpyv.push_back(PSpy);
		rpzv.push_back(PSpz);
		rKEv.push_back(PSKE);
		evtsRead++;
		
		// G4cout<< PSpartID << "\t" << PSposY << "\t" << PSposZ << "\t" <<  PSpx << "\t" << PSpy << "\t" << PSpz   << "\t" << PSKE << G4endl;
	}

	//G4cout << "Stored entries. Closing root file." << G4endl;
	f.Close(); 
}


RootParticleSource::~RootParticleSource()
{
}


void RootParticleSource::ResolveParameters() {
	TsVGenerator::ResolveParameters();
}


void RootParticleSource::GeneratePrimaries(G4Event* anEvent)
{
	if (CurrentSourceHasGeneratedEnough())
		return;
	 

TsPrimaryParticle p;

// random number to generate primary from
int RandNo = round(G4UniformRand()*NoOfHist);

// HIMAC defines beam direction as negative Y, and we want beam direction in z
// So we rotate it 90 degree on the Y axis to shoow in negative z

G4RotationMatrix rotationMatrix;
rotationMatrix.rotateX(90.0 * deg); // Rotate by 90 degrees around the Y axis

// Define momentum and positional vector of primary

// G4cout << rpxv[RandNo] << G4endl;

p.posX = ryPosv[RandNo] *mm;
p.posY = rzPosv[RandNo] *mm;
p.posZ = BeamPositionZ;

G4ThreeVector MVec(rpxv[RandNo], rpyv[RandNo], rpzv[RandNo]);
G4ThreeVector transformedMVec = rotationMatrix * MVec;

double magnitude = transformedMVec.mag();

p.dCos1 = transformedMVec.x() / magnitude;
p.dCos2 = transformedMVec.y() / magnitude;
p.dCos3 = transformedMVec.z() / magnitude; 
p.kEnergy =  rKEv[RandNo] *MeV;

G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
G4ParticleDefinition* particle = particleTable->FindParticle(rPartIDv[RandNo]);
p.particleDefinition = particle;
p.isNewHistory = 0;

// SetParticleType(p);
GenerateOnePrimary(anEvent, p);
AddPrimariesToEvent(anEvent);

}

bool RootParticleSource::isBothSpace(char const &lhs, char const &rhs) {
    return lhs == rhs && iswspace(lhs);
}