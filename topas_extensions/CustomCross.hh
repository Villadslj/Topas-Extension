//
// ********************************************************************
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring proton dosis over an LET threshold
// *                                                                  *
// ********************************************************************
//

#ifndef CustomCross_hh
#define CustomCross_hh


#include "TsVNtupleScorer.hh"
using namespace std;

class G4ParticleDefinition;

class CustomCross : public TsVNtupleScorer
{
public:
	CustomCross(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
								 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer=false);
	virtual ~CustomCross();

	G4bool ProcessHits(G4Step*,G4TouchableHistory*);

private:
	bool UseMaterialDensity;
	std::ifstream file;
	G4double CrossMinE;
	G4double CrossMaxE;
	G4double penergy;
	G4int Count;

	G4ParticleDefinition* fProtonDefinition;
	G4String Crossfile;
	G4double TargetConc;
	vector<double> row;
};

#endif
