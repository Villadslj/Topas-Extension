# Topas-Extension
This is a collection of different TOPAS extensions have made through the time. 
A describtion of how to use them and what they do can be found underneath: 

# How to include them in your Topas version

To include these (or other extensions) to your local Topas software you need to compile it in. 
It is described very well on the topas website https://sites.google.com/a/topasmc.org/home/user-guides/installation .
Follow step 5 (that is 5a to 5c).
I recommend to make your own bash script to do the compiling, since its not unlikley you will do this a lot every time you change an extension or want to include new ones. It would something like this: 

(For Debian based systems)
cd /<path to topas>/
cmake -DTOPAS_EXTENSIONS_DIR=/<path to this directory>/
make

# Dirty Dose.
The DirtyDose.cc (and .hh) scores the proton does for protons with LET values above a certain threshold. After it have been compiled in it can be used as follows.

Example of use:

s:Sc/DirtyDose/Quantity                       = "DirtyDose"
d:Sc/DirtyDose/LET_Max                        =  3000 MeV/mm/(g/cm3) # (optional)
d:Sc/DirtyDose/LET_Min                        =  2 MeV/mm/(g/cm3)
s:Sc/DirtyDose/Component                      = "SomeVolume" 
b:Sc/DirtyDose/OutputToConsole                = "FALSE" # (optional)
s:Sc/DirtyDose/IfOutputFileAlreadyExists      = "Overwrite" # (optional)
b:Sc/DirtyDose/PropagateToChildren = "True" # (optional)
s:Sc/DirtyDose/OutputFile = "Output/test/DirtyDose"

# Trip particle Generator
The TripParticleGenerator.cc (and .hh) generates particles from a .csv source files made in trip (simpel dose planing software, not for clinical use). 
But really any .csv file with the coulmns: 
*ENERGY(GEV) X(CM) Y(CM) FWHM(cm) WEIGHT 

should work. But really I would like it to take any generic file and look for these 5 parameters and the work. But for now it is .csv files.

Example of use:

s:So/MySource/Type  = "TripParticleGenerator"
s:So/MySource/Component  = "World"
i:So/MySource/NumberOfHistoriesInRun = 10000
s:So/MySource/ParticleSourceFile = "/PathToSourceFile/SourceFile.csv"
s:So/MySource/BeamParticle = "proton"


# Nuclear reaction scorer

This extension score the nuclear interactions that occure in a volume, and outputs .csv files including a set of the strings on the form:
A + B --> c + d. E.g:

protonInelastic : proton + B11 -->Li7 + alpha + proton

For now it only tracks protonInelastic and neutronInelastic reaktion, but it will be changed in the future. This is nice if you want to know what happens in your simulation and how different particles comes to be generated.

Example of use:
s:Sc/Ntuple/Quantity                       = "NuclearReaction"
s:Sc/Ntuple/OutputType                     = "ASCII"
s:Sc/Ntuple/Component                      = "SomeVolume"
b:Sc/Ntuple/OutputToConsole                = "TRUE" # (optional)
s:Sc/Ntuple/IfOutputFileAlreadyExists      = "Overwrite" # (optional)
b:Sc/Ntuple/PropagateToChildren = "True" # (optional)
s:Sc/Ntuple/OutputFile = "Output/test/Ntuple"

# General LET scorer

It does not work yet... 
But it should be able to score the LET of what ever particles you are interested in or just all of them. 
But for now it have to wait...
