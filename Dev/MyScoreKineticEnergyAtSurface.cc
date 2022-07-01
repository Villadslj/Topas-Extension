// Scorer for KineticEnergyAtSurface
//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * This file was obtained from Topas MC Inc under the license       *
// * agreement set forth at http://www.topasmc.org/registration       *
// * Any use of this file constitutes full acceptance of              *
// * this TOPAS MC license agreement.                                 *
// *                                                                  *
// ********************************************************************
//

#include "MyScoreKineticEnergyAtSurface.hh"

MyScoreKineticEnergyAtSurface::MyScoreKineticEnergyAtSurface(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
															 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{	
	SetSurfaceScorer();
	SetUnit("MeV");

	fNtuple->RegisterColumnD(&KE, "KineticEnergy", "MeV");
}


MyScoreKineticEnergyAtSurface::~MyScoreKineticEnergyAtSurface() {;}


G4bool MyScoreKineticEnergyAtSurface::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}
	ResolveSolid(aStep);
	G4cout << 'KE' << G4endl;
	if (IsSelectedSurface(aStep)) {
		KE = 1.;
		KE *= aStep->GetPreStepPoint()->GetWeight();
		G4cout << KE << G4endl;
		if ( KE > 0) {
		
			G4cout << KE << G4endl;
			fNtuple->Fill();

			return true;
		}
	}
	return false;
}
