//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For scoring nuclear reaction						              *
// *                                                                  *
// ********************************************************************
//

#ifndef CrossScorer_hh
#define CrossScorer_hh

#include "TsVNtupleScorer.hh"

class CrossScorer : public TsVNtupleScorer
{
public:
    CrossScorer(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);
    
    virtual ~CrossScorer();

    G4bool ProcessHits(G4Step*,G4TouchableHistory*);
    void ClearParameters();
    void FillEmptyParm();
    bool CheckSecondaries(std::vector<G4String>& v1, std::vector<G4String>& v2);

private:
	// Output variables	
	G4String projectile;
    G4String processname;
    G4String targetName;
    G4String secondaries;
    G4String pAlive;

    // User parameters
    G4String DTarget;
    G4String DProjectile;
    G4int slength;
    std::vector<G4String> DSecondaries;
    G4int slength2;
    std::vector<G4String> DSecondaries2;
};
#endif
