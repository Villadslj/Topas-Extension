//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring LET for a general particle or particles
// * incluing or excluding secondaries e.g.				              *
// *                                                                  *
// ********************************************************************
//

#ifndef MyScoreGeneralLET_hh
#define MyScoreGeneralLET_hh

#include "TsVBinnedScorer.hh"

class G4ParticleDefinition;

class MyScoreGeneralLET : public TsVBinnedScorer
{
public:
	MyScoreGeneralLET(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
				G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer=false);

	virtual ~MyScoreGeneralLET();
	G4bool ComputeLET(G4Step*,G4TouchableHistory*,G4ParticleDefinition* Particledef);
	G4bool ProcessHits(G4Step*,G4TouchableHistory*);
	G4int CombineSubScorers();

private:
	G4bool fDoseWeighted;
	G4bool fPreStepLookup;
	G4double fMaxScoredLET;
	G4double fNeglectSecondariesBelowDensity;
	G4double fUseFluenceWeightedBelowDensity;

	G4ParticleDefinition* Particledef;
	G4ParticleDefinition* fElectronDefinition;
	G4int fStepCount;
};
#endif
