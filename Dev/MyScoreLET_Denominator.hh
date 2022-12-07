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

#ifndef MyScoreLET_Denominator_hh
#define MyScoreLET_Denominator_hh

#include "TsVBinnedScorer.hh"

class G4ParticleDefinition;

class MyScoreLET_Denominator : public TsVBinnedScorer
{
public:
	MyScoreLET_Denominator(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer=false);
	virtual ~MyScoreLET_Denominator();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	G4bool fDoseWeighted;
	G4double fMaxScoredLET;
	G4double fNeglectSecondariesBelowDensity;
	G4double fUseFluenceWeightedBelowDensity;
	std::vector<G4ParticleDefinition*> p;

	G4ParticleDefinition* fProtonDefinition;
	G4ParticleDefinition* fElectronDefinition;
	G4int fStepCount;
	bool includeAll;
};

#endif
