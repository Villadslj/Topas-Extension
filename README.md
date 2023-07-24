# Topas-Extension
This is a collection of different TOPAS extensions have made through the time. 
A describtion of how to use them and what they do can be found underneath. Feel free to contribute code and your own cool extension to this project you like.

# How to include them in your TOPAS version

To include these (or other extensions) to your local TOPAS software you need to compile it in. 
It is described very well on the TOPAS website https://sites.google.com/a/topasmc.org/home/user-guides/installation .
Follow step 5 (that is 5a to 5c).
I recommend to make your own bash script to do the compiling, since its not unlikley you will do this every time you change an extension or want to include new ones. It would something like this: 

(For Debian based systems)
```
cd <path to TOPAS>
cmake -DTOPAS_EXTENSIONS_DIR=<path to this directory>
make
```
# Dirty Dose.
The DirtyDose.cc (and .hh) scores the proton does for protons with LET values above a certain threshold. After it have been compiled in it can be used as follows.

Example of use:
```
s:Sc/DirtyDose/Quantity                       = "DirtyDose"
d:Sc/DirtyDose/LET_Max                        =  3000 MeV/mm/(g/cm3) # (optional)
d:Sc/DirtyDose/LET_Min                        =  2 MeV/mm/(g/cm3)
s:Sc/DirtyDose/Component                      = "SomeVolume" 
b:Sc/DirtyDose/OutputToConsole                = "FALSE" # (optional)
s:Sc/DirtyDose/IfOutputFileAlreadyExists      = "Overwrite" # (optional)
b:Sc/DirtyDose/PropagateToChildren = "True" # (optional)
s:Sc/DirtyDose/OutputFile = "Path to output"
```
# Trip particle Generator
The TripParticleGenerator.cc (and .hh) generates particles from a .csv source files made in TRiP98 (simple research treatment planing software, not for clinical use). 
But really any .csv file with the coulmns: 
*ENERGY(GEV) X(CM) Y(CM) FWHM(cm) WEIGHT 

should work. But really I would like it to take any generic file and look for these 5 parameters and the work. But for now it is .csv files.

Example of use:
```
s:So/MySource/Type  = "TripParticleGenerator"
s:So/MySource/Component  = "World"
i:So/MySource/NumberOfHistoriesInRun = 10000
s:So/MySource/ParticleSourceFile = "PathToSourceFile"
s:So/MySource/BeamParticle = "proton"
```

# Nuclear reaction scorer

This extension score the nuclear interactions that occure in a volume, and outputs .csv files including a set of the strings on the form:
A + B --> c + d. E.g:

protonInelastic : proton + B11 -->Li7 + alpha + proton

For now it only tracks protonInelastic and neutronInelastic reaktion, but it will be changed in the future. This is nice if you want to know what happens in your simulation and how different particles comes to be generated.
```
Example of use:
s:Sc/Ntuple/Quantity                       = "NuclearReaction"
s:Sc/Ntuple/OutputType                     = "ASCII"
s:Sc/Ntuple/Component                      = "SomeVolume"
b:Sc/Ntuple/OutputToConsole                = "TRUE" # (optional)
s:Sc/Ntuple/IfOutputFileAlreadyExists      = "Overwrite" # (optional)
sv:Sc/Ntuple/Secondaries                   =  2 "neutorn", "alpha"
s:Sc/Ntuple/Target                         = "oxygen"
s:Sc/Ntuple/Projectile                     = "proton"
b:Sc/Ntuple/PropagateToChildren = "True" # (optional)
s:Sc/Ntuple/OutputFile = "Path to output"
```

# Qeff scoring

Scores the Qeff paramter which is an alternative RBE modelling parameter (instead of LETd). 
It is calculated as: 


```
G4double Energy = aTrack->GetKineticEnergy() / MeV;
G4double Mass = aTrack->GetParticleDefinition()->GetPDGMass() / MeV;
G4double beta = sqrt(1.0 - 1.0 / ( ((Energy / Mass) + 1) * ((Energy / Mass) + 1) ));
G4int z = aTrack->GetParticleDefinition()->GetAtomicNumber();
G4double Zeff = z * (1.0 - exp(-125.0 * beta * pow(abs(z), -2.0/3.0)));
G4double Qeff = Zeff*Zeff/(beta*beta);
```
Example off use in TOPAS is 

```
Example of use:
s:Sc/Qeff/Quantity                       = "Qeff"
s:Sc/Qeff/Component                      = "SomeVolume"
s:Sc/Qeff/WeightBy                   = "fluence" # or dose
s:Sc/Qeff/OutputFile =  "Qeff"
```
# Hadron LET scorer
Scores LET for all hadrons and not only protons
```
s:Sc/HadronTLET/Quantity                   = "myHadronLET"
s:Sc/HadronTLET/Component                  = "SomeVolume"
s:Sc/HadronTLET/WeightBy                   = "fluence" # or dose
s:Sc/HadronTLET/IfOutputFileAlreadyExists  = "Overwrite"
```
# Respons function weighted Ambient dose equivalence scorer

Scores Ambient dose equivalence but weighs the results with a specified detector response function. 
In examples folder you find a test script and a responsfunction from a WENDI-II neutron detector.

```
# Score ambient dose equivalent for neutrons
sv:Sc/scorerAmbDosePerSource/OnlyIncludeParticlesNamed = 1 "neutron"
s:Sc/scorerAmbDosePerSource/Quantity                   = "MyAmbientDoseEquivalent"
s:Sc/scorerAmbDosePerSource/Component                  = "WaterPhantom"
s:Sc/scorerAmbDosePerSource/OutputFile                 = "AmbientDoseNeutronPerSourceNeutron"
b:Sc/scorerAmbDosePerSource/OutputToConsole            = "True"
s:Sc/scorerAmbDosePerSource/IfOutputFileAlreadyExists  = "Overwrite"
s:Sc/scorerAmbDosePerSource/ImportRespons  = "Wendi2_Respons.csv"
sv:Sc/scorerAmbDosePerSource/Report                    = 2 "Sum" "Standard_Deviation"
# Set the fluence-to-dose conversion factors.
s:Sc/scorerAmbDosePerSource/GetAmbientDoseEquivalentForParticleNamed = "neutron"
dv:Sc/scorerAmbDosePerSource/FluenceToDoseConversionEnergies = 58
2.5314e-08 7.71605e-08 2.35195e-07 6.33404e-07 1.70582e-06 4.05885e-06 1.02746e-05 2.44475e-05 6.18866e-05 0.000142765
0.000309568 0.000611723 0.00100388 0.00150131 0.00217678 0.00305995 0.00430144 0.00604662 0.00849986  0.0119484
 0.0157877  0.0221931  0.0293242  0.0399651  0.0511969  0.0676476  0.0866593   0.101168     0.1296   0.171243
  0.233382   0.289858    0.37132   0.490632   0.590784   0.711379          1    1.85741    2.95521    4.95083
****
   9.98711    14.7825     18.937    20.1466    48.9368    98.7183    195.073    504.105    996.138    2182.41
   5086.78     9846.4      29400    99357.1     302853     982103 3.05600e+06 9.91011e+06  MeV
dv:Sc/scorerAmbDosePerSource/FluenceToDoseConversionValues   = 58
1.04694e-09 1.0279e-09 1.00922e-09 9.90868e-10 9.72854e-10 9.55168e-10 9.37803e-10 9.29239e-10 9.12346e-10 8.95759e-10
8.79474e-10 8.71443e-10 8.63485e-10 1.07615e-09 1.31681e-09 1.59657e-09 1.91809e-09 2.32559e-09 2.79393e-09 3.35658e-09
3.99571e-09 4.7131e-09 5.50853e-09 6.55742e-09 7.52478e-09 8.71443e-09 9.8182e-09 1.09608e-08 1.29287e-08 1.56754e-08
1.86602e-08 2.18095e-08 2.59623e-08 3.06236e-08 3.51412e-08 3.95922e-08 4.93432e-08 4.67006e-08 4.46069e-08 4.26071e-08
4.69871e-08 5.15014e-08 5.64495e-08 5.24551e-08 3.61218e-08 2.6443e-08 2.2418e-08 2.95202e-08 3.7933e-08 4.31315e-08
4.93432e-08 5.27769e-08 5.15014e-08 5.05651e-08 5.96437e-08 7.20944e-08 9.0956e-08 1.17594e-07 Sv*mm2
```
# Mixed Beam source
This is a beam source extension for simulating beams consisting of many different types of particles.
```
s:So/MySource/Type                     = "MixedSource"
s:So/MySource/Component                = "BeamPosition"
s:So/MySource/BeamParticle             = "proton"
sv:So/MySource/Particles               = 2 "proton" "neutron" # what particles to simulate
i:So/MySource/NumberOfHistoriesInRun   = 100000
uv:So/MySource/MixingFraction          = 2 0.9 0.1 # mixing fraction. This gives 90 % of the No. og histories is protons and 10 % is neutrons
dv:So/MySource/MonoEnergies             = 2 250 1 MeV # Energy of the particles. In this case the protons have 250 MeV and neutrons 1 MeV
d:So/MySource/BeamPositionZ= Ge/YourComponent/HLZ + 1.5 cm
```
