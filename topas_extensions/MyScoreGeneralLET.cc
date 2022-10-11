// Scorer for GeneralLET
//
// ********************************************************************
// *                                                                  *

// *                                                                  *
// ********************************************************************
//

#include "MyScoreGeneralLET.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmCalculator.hh"
#include "G4UIcommand.hh"

MyScoreGeneralLET::MyScoreGeneralLET(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("MeV/mm/(g/cm3)");

	
	fElectronDefinition = G4ParticleTable::GetParticleTable()->FindParticle("e-");
	fProtonDefinition = G4ParticleTable::GetParticleTable()->FindParticle("proton");

	fStepCount = 0;
	includeAll = false;
	// Check which particles to include in TOPAS
	G4String* includeparticles;
	if (fPm->ParameterExists(GetFullParmName("includeparticles"))){
		includeparticles = fPm->GetStringVector(GetFullParmName("includeparticles"));
		G4int length = fPm->GetVectorLength(GetFullParmName("includeparticles"));
		for (G4int i = 0; i < length; i++) {
			G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(includeparticles[i]);
			p.push_back(pdef);
		}

	}
	else {
		G4cout << "All particles are included in LET scoring";
		includeAll = true;
	}
		


	// Dose-averaged or fluence-averaged LET definition
	G4String weightType = "dose";
	if (fPm->ParameterExists(GetFullParmName("WeightBy")))
		weightType = fPm->GetStringParameter(GetFullParmName("WeightBy"));
	weightType.toLower();

	if (weightType == "dose") {
		fDoseWeighted = true;
	} else if (weightType == "fluence" || weightType == "track") {
		fDoseWeighted = false;
	} else {
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << GetFullParmName("WeightBy") << " refers to an unknown weighting: " << weightType << G4endl;
		exit(1);
	}

	// Upper cutoff to dE/dx (used to fix spikes). Neglected if zero/negative.
	if (fDoseWeighted) {
		fMaxScoredLET = 100 * MeV/mm/(g/cm3);  // default: 100 keV/um in water

		if (fPm->ParameterExists(GetFullParmName("MaxScoredLET"))) {
			fMaxScoredLET = fPm->GetDoubleParameter(GetFullParmName("MaxScoredLET"), "force per density");
		} else {  // make available to subscorer
			G4String transValue = G4UIcommand::ConvertToString(fMaxScoredLET / (MeV/mm/(g/cm3))) + " MeV/mm/(g/cm3)";
			fPm->AddParameter("d:" + GetFullParmName("MaxScoredLET"), transValue);
		}
	} else {
		fMaxScoredLET = 0;

		if (fPm->ParameterExists(GetFullParmName("MaxScoredLET"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetFullParmName("MaxScoredLET") << " is only available for dose-weighted LET scorers." << G4endl;
			exit(1);
		}
	}
	// Neglect secondary electrons in low density materials where the mean path length between discrete processes is
	// longer than the voxel width. Otherwise the LET is too high.
	if (fDoseWeighted) {
		fNeglectSecondariesBelowDensity = 0.1 * g/cm3;

		if (fPm->ParameterExists(GetFullParmName("NeglectSecondariesBelowDensity"))) {
			fNeglectSecondariesBelowDensity = fPm->GetDoubleParameter(GetFullParmName("NeglectSecondariesBelowDensity"), "Volumic Mass");
		} else {  // make available to subscorer
			G4String transValue = G4UIcommand::ConvertToString(fNeglectSecondariesBelowDensity / (g/cm3)) + " g/cm3";
			fPm->AddParameter("d:" + GetFullParmName("NeglectSecondariesBelowDensity"), transValue);
		}
	} else {
		fNeglectSecondariesBelowDensity = 0;

		if (fPm->ParameterExists(GetFullParmName("NeglectSecondariesBelowDensity"))) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetFullParmName("NeglectSecondariesBelowDensity") << " is only available for dose-weighted LET scorers." << G4endl;
			exit(1);
		}
	}
	// Use fluence-weighted LET for low density materials in order to avoid rare spikes (a.k.a. pretty plot mode)
	// Disabled by default
	fUseFluenceWeightedBelowDensity = 0;
	if (fPm->ParameterExists(GetFullParmName("UseFluenceWeightedBelowDensity"))) {
		if (!fDoseWeighted) {
			G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
			G4cerr << GetFullParmName("UseFluenceWeightedBelowDensity") << " is only available for dose-weighted LET scorers." << G4endl;
			exit(1);
		}
		fUseFluenceWeightedBelowDensity = fPm->GetDoubleParameter(GetFullParmName("UseFluenceWeightedBelowDensity"), "Volumic Mass");
	}

	// Instantiate subscorer needed for denominator
	InstantiateSubScorer("LET_Denominator", outFileName, "Denominator");
}

MyScoreGeneralLET::~MyScoreGeneralLET() {;}



G4bool MyScoreGeneralLET::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	// Checks if particle in scoring volume is part of the lists of desired particles to score LET
	const G4ParticleDefinition* PDef = aStep->GetTrack()->GetParticleDefinition();
	if (includeAll == false){
		if (std::find(p.begin(), p.end(), PDef) != p.end()){
		}
		else {
			return false;
		}

	}


	// Get the energy deposit and Density of material
	G4double stepLength = aStep->GetStepLength();
	if (stepLength <= 0.)
		return false;

	G4double eDep = aStep->GetTotalEnergyDeposit();
	G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();

	G4bool isStepFluenceWeighted = !fDoseWeighted || density < fUseFluenceWeightedBelowDensity;

	// Add energy deposited by secondary electrons (unless NeglectSecondariesBelowDensity is used)
	if (isStepFluenceWeighted || density > fNeglectSecondariesBelowDensity) {
		const G4TrackVector* secondary = aStep->GetSecondary();
		if (!secondary) {
			secondary = aStep->GetTrack()->GetStep()->GetSecondary();  // parallel worlds
		}
		if (secondary) {
			G4int diff;
			if (fStepCount == 0) diff = 0;
			else diff = secondary->size() - fStepCount;

			fStepCount = (*secondary).size();

			for (unsigned int i=(*secondary).size()-diff; i<(*secondary).size(); i++)
				if ((*secondary)[i]->GetParticleDefinition() == fElectronDefinition)
					eDep += (*secondary)[i]->GetKineticEnergy();
		}
	}

	// Compute LET
	G4double dEdx = 0;
	if (fPreStepLookup) {
		G4EmCalculator emCal;
		G4double preStepKE = aStep->GetPreStepPoint()->GetKineticEnergy();
		dEdx = emCal.ComputeElectronicDEDX(preStepKE, PDef, aStep->GetPreStepPoint()->GetMaterial());
	} else {
		dEdx = eDep / stepLength;
	}

	// Compute weight (must be unitless in order to use a single denominator sub-scorer)
	G4double weight = 1.0;
	if (isStepFluenceWeighted)
		weight *= (stepLength/mm);
	else
		weight *= (eDep/MeV);

	// If dose-weighted and not using PreStepLookup, only score LET if below MaxScoredLET
	// Also must check if fluence-weighted mode has been enabled by a low density voxel
	if (isStepFluenceWeighted || fMaxScoredLET <= 0 || dEdx / density < fMaxScoredLET) {
		AccumulateHit(aStep, weight * dEdx / density);
		return true;
	}
	
	return false;
}

G4int MyScoreGeneralLET::CombineSubScorers()
{
	G4int counter = 0;

	TsVBinnedScorer* denomScorer = dynamic_cast<TsVBinnedScorer*>(GetSubScorer("Denominator"));
	for (G4int index=0; index < fNDivisions; index++) {
		if (denomScorer->fFirstMomentMap[index]==0.) {
			fFirstMomentMap[index] = 0;
			counter++;
		} else {
			fFirstMomentMap[index] = fFirstMomentMap[index] / denomScorer->fFirstMomentMap[index];
		}
	}

	return counter;
}
