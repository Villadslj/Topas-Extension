//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring proton dosis over an LET threshold
// *                                                                  *
// ********************************************************************
//

#ifndef Qeff_hh
#define Qeff_hh

#include "TsVBinnedScorer.hh"

class G4ParticleDefinition;

class Qeff : public TsVBinnedScorer
{
public:
	Qeff(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer=false);
	virtual ~Qeff();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	G4double fMaxScoredLET;
	G4double fMinScoredLET;
	G4double fNeglectSecondariesBelowDensity;

	G4ParticleDefinition* fProtonDefinition;
	G4ParticleDefinition* fElectronDefinition;
	G4int fStepCount;
};

#endif
