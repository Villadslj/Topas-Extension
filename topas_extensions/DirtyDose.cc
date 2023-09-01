// Scorer for DirtyDose
//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring proton dosis over an LET threshold
// *                                                                  *
// ********************************************************************
//

#include "DirtyDose.hh"

#include "TsParameterManager.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"


DirtyDose::DirtyDose(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
														   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
	:TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("");

	fProtonDefinition = G4ParticleTable::GetParticleTable()->FindParticle("proton");
	fElectronDefinition = G4ParticleTable::GetParticleTable()->FindParticle("e-");

	fStepCount = 0;

	if (!fPm->ParameterExists(GetFullParmName("LET_Max"))){
		fMaxScoredLET = 0;
	} else {
		fMaxScoredLET = fPm->GetDoubleParameter(GetFullParmName("LET_Max"), "force per density");
	}

	if (!fPm->ParameterExists(GetFullParmName("LET_Min"))){
		G4cerr << "Topas is exiting due to a serious error in scoring setup." << G4endl;
		G4cerr << "LET_Min parameter is missing " << G4endl;
		exit(1);
	} else {
		fMinScoredLET = fPm->GetDoubleParameter(GetFullParmName("LET_Min"), "force per density");
	}
	
	// Neglect secondary electrons in low density materials where the mean path length between discrete processes is
	// longer than the voxel width. Otherwise the LET is too high.
	fNeglectSecondariesBelowDensity = 0;
	if (fPm->ParameterExists(GetFullParmName("NeglectSecondariesBelowDensity")))
		fNeglectSecondariesBelowDensity = fPm->GetDoubleParameter(GetFullParmName("NeglectSecondariesBelowDensity"), "Volumic Mass");


}


DirtyDose::~DirtyDose()
{;}


G4bool DirtyDose::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	if (aStep->GetTrack()->GetParticleDefinition()==fProtonDefinition) {

		G4double stepLength = aStep->GetStepLength();
		if (stepLength <= 0.)
			return false;

		G4double eDep = aStep->GetTotalEnergyDeposit();
		G4double density = aStep->GetPreStepPoint()->GetMaterial()->GetDensity();




		// Add energy deposited by secondary electrons (unless NeglectSecondariesBelowDensity is used)
		if (density > fNeglectSecondariesBelowDensity) {
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


		// Compute dose
		ResolveSolid(aStep); // Dont know what it does
		G4double dose = eDep / ( density * fSolid->GetCubicVolume() );
		dose *= aStep->GetPreStepPoint()->GetWeight(); // dont know what it does ?

		// Compute LET
		G4double dEdx = eDep / stepLength;
		
		// Only dosis if LET is below LET_Max
		if (fMaxScoredLET != 0){
			if  (dEdx / density < fMaxScoredLET && dEdx / density > fMinScoredLET) {
				AccumulateHit(aStep, dose);
				return true;
			}

		}
		else {
			if  (dEdx / density > fMinScoredLET) {
				AccumulateHit(aStep, dose);
				return true;
			}

		}

	}
	return false;
}
