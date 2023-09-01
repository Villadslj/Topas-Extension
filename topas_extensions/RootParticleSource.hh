//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//
#ifndef RootParticleSource_hh
#define RootParticleSource_hh

#include "TsVGenerator.hh"
#include <G4ParticleGun.hh>
#include <fstream>
#include <CLHEP/Random/RandFlat.h>
#include <random>
#include "TFile.h"
#include "TNtuple.h"
#include "TTree.h"
#include <vector>


class RootParticleSource : public TsVGenerator
{
public:
	RootParticleSource(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~RootParticleSource();

	void ResolveParameters();
	bool isBothSpace(char const &lhs, char const &rhs);

	void GeneratePrimaries(G4Event* );

	#define MyRandNGenerator() CLHEP::HepRandom::getTheEngine()->flat()
	
private:

	std::vector<std::vector<std::string>> parsedCsv;
	std::default_random_engine generator;
	G4double BeamPositionZ;
	G4String ParticleSourceFile;
	G4int linemax, NoOfHist;

	G4int numberEntries;
	std::vector<G4int> rPartIDv;
	std::vector<G4int> rAAv;
	std::vector<G4int> rZZv;
	std::vector<G4double> rxPosv;
	std::vector<G4double> ryPosv;
	std::vector<G4double> rzPosv;
	std::vector<G4double> rpxv;
	std::vector<G4double> rpyv;
	std::vector<G4double> rpzv;
	std::vector<G4double> rKEv;
	
	
};
#endif