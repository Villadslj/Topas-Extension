// Scorer for Qeff
//
// ********************************************************************
// *                                                                  *

// *                                                                  *
// ********************************************************************
//

#include "Qeff.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmCalculator.hh"
#include "G4UIcommand.hh"

Qeff::Qeff(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");


	fStepCount = 0;

	// Dose-averaged or fluence-averaged Qeff definition
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

	// Upper cutoff to Qeff (used to fix spikes). Neglected if zero/negative.
	if (fPm->ParameterExists(GetFullParmName("MaxScoredQeff"))) {
		fMaxScoredQeff = fPm->GetDoubleParameter(GetFullParmName("MaxScoredQeff"), "");
	} 
	

	// Instantiate subscorer needed for denominator
	InstantiateSubScorer("Qeff_Denominator", outFileName, "DenominatorQ");
}

Qeff::~Qeff() {;}



G4bool Qeff::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	// Checks if particle is an ion.
	G4int z = aStep->GetTrack()->GetParticleDefinition()->GetAtomicNumber();
	if (z < 1 || z == NULL){
		return false;

	}


	// Get the energy deposit and Density of material
	G4double stepLength = aStep->GetStepLength();
	if (stepLength <= 0.)
		return false;

	G4double eKin = aStep->GetTrack()->GetKineticEnergy();
	G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();
	G4bool isStepFluenceWeighted = !fDoseWeighted;


	// Compute Qeff
	auto aTrack = aStep->GetTrack();
	G4double Energy = aTrack->GetKineticEnergy();
	G4double Mass = aTrack->GetParticleDefinition()->GetPDGMass();
	G4double beta = sqrt(1.0 - 1.0 / ( ((Energy / Mass) + 1) * ((Energy / Mass) + 1) ));
	G4double Zeff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), -2.0/3.0)));
	G4double Qeff = Zeff*Zeff/(beta*beta);

	

	// Compute weight (must be unitless in order to use a single denominator sub-scorer)
	G4double weight = 1.0;
	if (isStepFluenceWeighted)
		weight *= (stepLength/mm);
	else
		weight *= (eKin/MeV);

	// If dose-weighted and not using PreStepLookup, only score Qeff if below MaxScoredQeff
	// Also must check if fluence-weighted mode has been enabled by a low density voxel
	// G4cout << Qeff << G4endl;
	if (!isStepFluenceWeighted  || Qeff < fMaxScoredQeff) {
		AccumulateHit(aStep, weight * Qeff / density);
		return true;
	}
	
	return false;

}

G4int Qeff::CombineSubScorers()
{
	G4int counter = 0;

	TsVBinnedScorer* denomScorer = dynamic_cast<TsVBinnedScorer*>(GetSubScorer("DenominatorQ"));
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