//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring Qeff for a general particle or particles
// * incluing or excluding secondaries e.g.				              *
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
	G4int CombineSubScorers();

private:
	G4bool fDoseWeighted;
	G4bool fPreStepLookup;
	G4double fMaxScoredQeff;
	G4double fNeglectSecondariesBelowDensity;
	G4double fUseFluenceWeightedBelowDensity;
	G4int Order;
	std::vector<G4ParticleDefinition*> p;

	bool includeAll;
	G4ParticleDefinition* Particledef;
	G4ParticleDefinition* fElectronDefinition;
	G4ParticleDefinition* fProtonDefinition;
	G4int fStepCount;
};
#endif