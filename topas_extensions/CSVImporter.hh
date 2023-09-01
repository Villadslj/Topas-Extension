//
// ********************************************************************
// *                                                                  *
// *                                                                  *
// * Created by Villads J. 2022                                       *
// * For including source files from trip                             *
// *                                                                  *
// ********************************************************************
//
#ifndef CSVImporter_hh
#define CSVImporter_hh

#include "TsVGenerator.hh"
#include <G4ParticleGun.hh>
#include <fstream>
#include <CLHEP/Random/RandFlat.h>
#include <random>

class CSVImporter : public TsVGenerator
{
public:
	CSVImporter(TsParameterManager* pM, TsGeometryManager* gM, TsGeneratorManager* pgM, G4String sourceName);
	~CSVImporter();

	void ResolveParameters();
	bool isBothSpace(char const &lhs, char const &rhs);

	void GeneratePrimaries(G4Event* );

	#define MyRandNGenerator() CLHEP::HepRandom::getTheEngine()->flat()
	
private:

	// Define vectors to store each column
    std::vector<double> col1;
    std::vector<double> col2;
    std::vector<double> col3;
    std::vector<double> col4;
    std::vector<double> col5;
    std::vector<double> col6;
    std::vector<double> col7;
    std::vector<double> col8;
	std::default_random_engine generator;
	G4double BeamPositionZ;
	G4String ParticleSourceFile;
	G4int linemax;
	
	
};
#endif