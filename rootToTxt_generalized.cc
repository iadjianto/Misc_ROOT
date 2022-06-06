#include <vector>
#include <iostream>
#include <fstream>
#include <TRandom3.h>
#include "TChannel.h"
#include <tuple>

/*
INPUT ARGUMENTS:
filename - name of .root file to be opened
objth2d - name of TH2D object to be opened

OUTPUT:
text file with axes laid out as:
----------------------> x
|
|
|
|
|
|
|
V

y

each (x,y) cell is filled in with the number of counts and the center of the y bin, which is assumed to be energy.

*/

void GetSpectra(const char* filename = "EnergyCalib_Rb92_Sum.root", const char* objth2d= "gCA"){	// filename of file to be read and the name of the TH2D object to be read in
	TFile *fSpectra = TFile::Open(filename,"READ"); // File to be opened
	ofstream outfile;
	outfile.open("Calib.dat", ios::out | ios::trunc );
	TH2D *matrix = (TH2D*)fSpectra->Get(objth2d); // object to be read
	int x_NBins = matrix->GetXaxis()->GetNbins();
	int y_NBins = matrix->GetYaxis()->GetNbins();
	double bin_content;
	double bin_center;
	for(int j = 0; j < y_NBins; j++){
		for(int i=0; i < x_NBins; i++){
			bin_content = matrix->GetBinContent(i,j);// Counts within the (x,y) bin
			bin_center = matrix->GetYaxis()->GetBinCenter(i);// CAN COMMENT OUT - used to get center of y bin, useful for energy bins.
			
			if(i!=x_NBins-1){
				outfile<<bin_center<<","<<bin_content<<",";
			}
			else{
				outfile<<bin_center<<","<<bin_content;
			}
			
		}
		outfile << "\n";
		
	}
	
}
