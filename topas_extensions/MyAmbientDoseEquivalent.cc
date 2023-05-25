// Scorer for MyAmbientDoseEquivalent
//
// ********************************************************************
// *                                                                  *
// * Copyright 2022 The TOPAS Collaboration                           *
// *                                                                  *
// * Permission is hereby granted, free of charge, to any person      *
// * obtaining a copy of this software and associated documentation   *
// * files (the "Software"), to deal in the Software without          *
// * restriction, including without limitation the rights to use,     *
// * copy, modify, merge, publish, distribute, sublicense, and/or     *
// * sell copies of the Software, and to permit persons to whom the   *
// * Software is furnished to do so, subject to the following         *
// * conditions:                                                      *
// *                                                                  *
// * The above copyright notice and this permission notice shall be   *
// * included in all copies or substantial portions of the Software.  *
// *                                                                  *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  *
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES  *
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND         *
// * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT      *
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,     *
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR    *
// * OTHER DEALINGS IN THE SOFTWARE.                                  *
// *                                                                  *
// ********************************************************************
//

#include "MyAmbientDoseEquivalent.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>

MyAmbientDoseEquivalent::MyAmbientDoseEquivalent(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
							   G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVBinnedScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer)
{
	SetUnit("Sv");

	if (fPm->ParameterExists(GetFullParmName("ImportRespons"))){
		Responsfile = fPm->GetStringParameter(GetFullParmName("ImportRespons"));
	}


    // Read data from file and create bibliography
    std::ifstream file(Responsfile);
    if (file.is_open()) {

		std::string line;
		while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string x_str, y_str;
        if (std::getline(iss, x_str, ',') && std::getline(iss, y_str)) {
            double x = std::stod(x_str);
            double y = std::stod(y_str);
        	bibliography[x] = y;
        	}
    	}
    } else {
        std::cerr << "Unable to open file." << std::endl;
		fPm->AbortSession(1);
    }


	G4String particleName = fPm->GetStringParameter(GetFullParmName("GetAmbientDoseEquivalentForParticleNamed"));
	fParticleDefinition = G4ParticleTable::GetParticleTable()->FindParticle(particleName);

	fEnergies = fPm->GetDoubleVector(GetFullParmName("FluenceToDoseConversionEnergies"), "Energy");
	fCoefficients = fPm->GetDoubleVector(GetFullParmName("FluenceToDoseConversionValues"), "dose fluence");
	fBins = fPm->GetVectorLength(GetFullParmName("FluenceToDoseConversionEnergies"));
	if ( fBins != fPm->GetVectorLength(GetFullParmName("FluenceToDoseConversionValues"))) {
		G4cout << "TOPAS is exiting due to error in scoring setup." << G4endl;
		G4cout << "Scorer " << GetName() << " has quantity AmbientDoseEquivalent" << G4endl;
		G4cout << "but vector length of the conversion factor and energies mismatch." << G4endl;
		fPm->AbortSession(1);
	}
}


MyAmbientDoseEquivalent::~MyAmbientDoseEquivalent() {;}


G4bool MyAmbientDoseEquivalent::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}

	G4double quantity = aStep->GetStepLength();

	if ( quantity > 0. && aStep->GetTrack()->GetParticleDefinition()==fParticleDefinition) {
		ResolveSolid(aStep);
		G4double ekin = aStep->GetTrack()->GetKineticEnergy();

		quantity /= GetCubicVolume(aStep);
		quantity *= aStep->GetPreStepPoint()->GetWeight();

		G4double coefficient = 0;
		for ( int i = fBins-1; i >= 0; i-- ) {
			if ( ekin >= fEnergies[i] ) {
				coefficient = fCoefficients[i] + ( fCoefficients[i+1] - fCoefficients[i] ) * ( ekin - fEnergies[i] ) / ( fEnergies[i+1] - fEnergies[i] );
				break;
			}
		}
		quantity *= coefficient;

		double closest_key = findClosestKey(bibliography, ekin);
    	double weight = bibliography[closest_key];

		G4cout <<closest_key << " " << ekin << " " << weight << G4endl;

		AccumulateHit(aStep, quantity*weight);

		return true;
	}
	return false;
}

G4double MyAmbientDoseEquivalent::findClosestKey(const std::map<double, double>& dictionary, double target) {
    auto closestKey = dictionary.begin();
    double minDifference = std::abs(target - closestKey->first);

    for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
        double difference = std::abs(target - it->first);
        if (difference < minDifference) {
            minDifference = difference;
            closestKey = it;
        }
    }

    return closestKey->first;
}
