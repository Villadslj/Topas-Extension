// Scorer for CustomCross
//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring proton dosis over an LET threshold
// *                                                                  *
// ********************************************************************
//

#include "CustomCross.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include <random>
#include <iostream>
#include "Randomize.hh"

using namespace std;

CustomCross::CustomCross(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
														   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
	:TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");
	fNtuple->RegisterColumnD(&penergy, "parentE", "MeV");
	fNtuple->RegisterColumnI(&Count, "Count");
	fProtonDefinition = G4ParticleTable::GetParticleTable()->FindParticle("proton");

	if (!fPm->ParameterExists(GetFullParmName("ImportCross"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "ImportCross parameter is missing... And you are an absolute donky ";
		exit(1);
	} else {
		Crossfile = fPm->GetStringParameter(GetFullParmName("ImportCross"));
	}

	UseMaterialDensity = false;
	if (!fPm->ParameterExists(GetFullParmName("TargetConc"))){
		UseMaterialDensity = true;
	} else {
		TargetConc = fPm->GetUnitlessParameter(GetFullParmName("TargetConc"));
	}

	file.open(Crossfile);

	string line, csvItem;
	int lineNumber = 0;
	if (file.is_open()) {
		bool crossfound = false; 
		while (getline(file,line)) {
			lineNumber++;
			istringstream myline(line);
			while(getline(myline, csvItem, '	')) {
				row.push_back(stod(csvItem));
			}
		}
	}
	CrossMinE = row[0];
	CrossMaxE = row[row.size() - 2];
	G4cout << "CrossMinE is " << CrossMinE << G4endl;
	G4cout << "CrossMaxE is " << CrossMaxE << G4endl;
	file.close();

}
CustomCross::~CustomCross()
{;}


G4bool CustomCross::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}
	ResolveParameters();
	Count = 0;
	if (aStep->GetTrack()->GetParticleDefinition()==fProtonDefinition) {
		
		// Get particle energy
		penergy = aStep->GetTrack()->GetKineticEnergy() * MeV;
		if (penergy < CrossMinE || penergy > CrossMaxE) return false;
	
		int index;

		for(int i = 0; i < row.size(); i+=2){
			auto energydiff = abs(row[i] - penergy);
			if(abs(row[i + 2] - penergy) > energydiff) {
				index = i;
				break;
			}
		}
		// Find cross section for that energy
		auto Cross = row[index + 1];
		// if (!Cross == 100)G4cout << "What?? " << endl;
		// auto Cross = 50;
		
		// Find steplength
		G4double stepLength = aStep->GetStepLength() / cm;
		if (stepLength <= 0.)
			return false;

		// Calculate probability of reaction
		if (UseMaterialDensity == true){
			auto Material = aStep->GetTrack()->GetMaterial();
			TargetConc = Material->GetTotNbOfAtomsPerVolume()/(1/cm3);

		}
		// G4cout << stepLength << G4endl;

		double nb = TargetConc * stepLength * 1.E-24; // pow(10,-24) is converstion from atoms/cm⁻2 to atoms/barns
		double propRea = nb * Cross;
		if (propRea > 1){
			G4cout << "propbabillity to high!" << G4endl;
		}
		// G4cout << nb << G4endl;
		//G4cout << propRea << G4endl;
		auto ReaTest = G4UniformRand();
		if(ReaTest < propRea){
			Count = 1;
			fNtuple->Fill();
			aStep->GetTrack()->SetTrackStatus(fStopAndKill);
			return true;
		}

		return false;
	}
	return false;
}
