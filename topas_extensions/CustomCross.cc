// Scorer for CustomCross
//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring reaction yield for a specified Cross section         *
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
	:TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");

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
		if (!fPm->ParameterExists(GetFullParmName("ReactionElement"))){
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << "ReactionElement or TargetConc parameter is missing... And you are an absolute donky ";
			exit(1);
		} else {
			UseMaterialDensity = true;
			ReactionElement = fPm->GetStringParameter(GetFullParmName("ReactionElement"));
		}

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
			while(getline(myline, csvItem, ',')) {
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
		penergy = aStep->GetTrack()->GetKineticEnergy() / MeV;
		if (penergy < CrossMinE || penergy > CrossMaxE) return false;
		
		// Find cross section for that energy
		int index;

		for(int i = 0; i < row.size(); i+=2){
			auto energydiff = abs(row[i] - penergy);
			if(abs(row[i + 2] - penergy) > energydiff) {
				index = i;
				break;
			}
		}

		double Cross = row[index +1];
		// double Cross = 1;
		
		// Find steplength
		G4double stepLength = aStep->GetStepLength() / m;
		if (stepLength <= 0.)
			return false;

		// Get number of atoms per volume for the desired element
		if (UseMaterialDensity == true){
			auto Material = aStep->GetTrack()->GetMaterial();
			auto Ev = Material->GetElementVector();
			auto NElement = Material->GetNumberOfElements();
			auto Vna = Material->GetVecNbOfAtomsPerVolume();
			// G4cout << NElement << G4endl;
			for (G4int i = 0; i < NElement; i++){
				auto name = Material->GetElement(i)->GetName();
				if (name == ReactionElement) {
				TargetConc = Vna[i] /(1/m3);
				}

			}
		}
			

		// Calculate probability of reaction
		double Sigma = TargetConc * Cross * 1.e-28;
		double propRea = 1 - exp(-Sigma*stepLength);

		double ReaTest = G4UniformRand();
		if(ReaTest < propRea){
			Count += 1;
			AccumulateHit(aStep, Count);
			aStep->GetTrack()->SetTrackStatus(fStopAndKill);
			return true;
		}

		return false;
	}
	return false;
}
