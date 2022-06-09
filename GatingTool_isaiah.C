#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include <iostream>
#include <TSystem.h>
#include <chrono>
#include <ctime>

using namespace std;

//branches
string start = "La1_energy";
string stop = "La2_energy";
string tac = "TAC_energy";
string HPGe1 = "Ge_energy";
//string HPGe2 = "";

TFile *myfile;
TChain llt("LaLaTac_tree");
TChain llgt("LaLaGeTac_tree");

TH1D *gsE = new TH1D("gsE","Griffin Projection g-l-l coincidences",5000,0.5, 5000.5); // Projection goes unused, possible gating target.
TH1D *htemp;
TH2D *lEA;

TFile *outfile = TFile::Open("isaiah_analysis.root","UPDATE");

TTree *start_tree;
TTree *stop_tree;

/*
PURPOSE:
- Outputs syntax for ROOT to gate around certain bins
OUTPUT:
- const char* that can be used by ROOT to draw gates from TTrees
*/
const char *GatingCondition(string branch, int low, int high){
	const char *gate_condition;
	gate_condition = Form("%s>%d&&%s<%d",branch.c_str(),low, branch.c_str(), high );
	return gate_condition;
}

/*
PURPOSE:
- Reads in .root files, sums them together into a TTree (you can treat this as a TFile)
OUTPUT:
- Creates TH1D of LaBr projection gsE object
*/
void SumTrees(int FirstRum, int LastRun){
	
	int counter=0;
	for(int run_number = FirstRum; run_number <= LastRun; run_number++){
		string temp = Form("56Co/FastTimingData_%d_000-001.root",run_number);	
		if( !gSystem->AccessPathName( temp.c_str() ) ){
			myfile = TFile::Open( temp.c_str() );
			htemp =  (TH1D*)myfile->Get("gsE");
			gsE->Add(htemp);
			myfile->Close();
			llt.Add(temp.c_str());
			llgt.Add(temp.c_str());
			counter++;
		}	
	}
	cout << "Total Number of files addad = " << counter << endl;
}

void DrawGRIFFIN(){
	gsE->Draw();
}

void MakeLaLaEnergyMatrix(int NBins, double firstbin, double lastbin){
	
	outfile->cd();
	llt.Draw(Form("%s:%s>>lltE(%d,%f,%f,%d,%f,%f)",stop.c_str(),start.c_str(),NBins,firstbin,lastbin,NBins,firstbin,lastbin));
	TH2D *lltE = (TH2D*)outfile->Get("lltE");
}

/*
PURPOSE:
- Applies energy gate to first LaBr
OUTPUT:
- Creates TH2D of second LaBr and TAC bins. 
- Creates TH1D of second LaBr projection if user wants to visualize possible gating targets
*/
//TH2D *llt_start_gate(int La1_Low, int La1_high,int LaBrNBins = 5000, double LaBrfirstbin = 0.5, double LaBrlastbin = 5000.5, int TacNBins = 50000, double Tacfirstbin = 0, double Taclastbin = 50000){
void llt_start_gate(int La1_Low, int La1_high,int LaBrNBins = 5000, double LaBrfirstbin = 0.5, double LaBrlastbin = 5000.5, int TacNBins = 50000, double Tacfirstbin = 0, double Taclastbin = 50000){
	outfile->cd();

	//start_tree->Clear();
	start_tree = llt.CopyTree( GatingCondition(start,La1_Low, La1_high ) );
	start_tree->Draw(Form("%s:%s>>latac_gate_la1_%d_%d(%d,%f,%f,%d,%f,%f)",stop.c_str(),tac.c_str(),La1_Low,La1_high,TacNBins,Tacfirstbin,Taclastbin,LaBrNBins,LaBrfirstbin,LaBrlastbin));
	TH2D *latacdelmat = (TH2D*)outfile->Get(Form("latac_gate_la1_%d_%d",La1_Low,La1_high));
	TH1D *laprojhisto = latacdelmat->ProjectionY(Form("la2_proj_gate_la1_%d_%d",La1_Low,La1_high));
	start_tree->SetName("StartTree");
	
	
	stop_tree = llt.CopyTree( GatingCondition(stop,La1_Low,La1_high ) );
	stop_tree->Draw(Form("%s:%s>>latac_gate_la2_%d_%d(%d,%f,%f,%d,%f,%f)",start.c_str(),tac.c_str(),La1_Low,La1_high,TacNBins,Tacfirstbin,Taclastbin,LaBrNBins,LaBrfirstbin,LaBrlastbin));
	TH2D *latacantidelmat = (TH2D*)outfile->Get(Form("latac_gate_la2_%d_%d",La1_Low,La1_high));
	TH1D *laprojantihisto = latacantidelmat->ProjectionY(Form("la1_proj_gate_la2_%d_%d",La1_Low,La1_high));
	stop_tree->SetName("StopTree");

	// This section is to write the histograms to a file if necessary
	outfile->Write(latacdelmat->GetName(),TObject::kOverwrite);
	//outfile->Write(laprojhisto->GetName(),TObject::kOverwrite);
	outfile->Write(latacantidelmat->GetName(),TObject::kOverwrite);
	//outfile->Write(laprojantihisto->GetName(),TObject::kOverwrite);

	//TH2D* latac_mats[2] = {latacdelmat,latacantidelmat};return latac_mats;
	
}

/*
PURPOSE:
- Applies second energy gate to second LaBr
OUTPUT:
- Creates TH1D of TAC bins, difference between start_tree and stop_tree results in the centroid difference necessary
FUTURE:
- possibly allow this function to be used independently of using llt_start_gate() first, maybe by calling it with an if condition
*/

void Draw_llt_TacSpectra(int La1_Low, int La1_high, int La2_Low, int La2_high, int TacNBins = 50000, double Tacfirstbin = 0, double Taclastbin = 50000){
	outfile->cd();

	TH2D *latac_del_mat = (TH2D*)outfile->Get(Form("latac_gate_la1_%d_%d",La1_Low,La1_high));
	TH1D *histoTAC_del = latac_del_mat->ProjectionX(Form("hTAC_La1_%d_%d_La2_%d_%d",La1_Low,La1_high,La2_Low,La2_high),La2_Low,La2_high);
	
	TH2D *latac_antidel_mat = (TH2D*)outfile->Get(Form("latac_gate_la2_%d_%d",La1_Low,La1_high));
	TH1D *histoTAC_antidel = latac_antidel_mat->ProjectionX(Form("hTAC_La1_%d_%d_La2_%d_%d",La2_Low,La2_high,La1_Low,La1_high),La1_Low,La1_high);

	// This section is to write the histograms to a file if necessary
	outfile->Write(histoTAC_del->GetName(),TObject::kOverwrite);
	outfile->Write(histoTAC_antidel->GetName(),TObject::kOverwrite);
}
