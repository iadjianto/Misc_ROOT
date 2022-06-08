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

TFile *outfile = TFile::Open("myanalysis.root","RECREATE");

tuple<double, double, double, double, TPeakFitter*, TRWPeak*> FitResult(TH1D *histogram, double cent_val, double low_bound, double up_bound){
	TPeakFitter *peak_fit = new TPeakFitter(low_bound,up_bound);
	TRWPeak *peak = new TRWPeak(cent_val);
	peak_fit->AddPeak(peak);
	peak_fit->Fit(histogram,"EQLS+");
	double new_cent_val = peak->GetFitFunction()->GetParameter(1);
	double error = peak->GetFitFunction()->GetParError(1);
	double width = peak->GetFitFunction()->GetParameter(2);
	double chi2 = peak->GetFitFunction()->GetParameter(0) / peak_fit->GetFitFunction()->GetNDF();
	return {new_cent_val, error, width, chi2, peak_fit, peak};
}

void MyFits( int DetNum ){  // Fit peaks chosen detector

	SpectraFile->cd();
	if( hist[i][j]->Integral() < 10000 ) continue; //skipping empty spectra
	for(int k = 0; k < LitEnergy[i].size(); k++){
		cout << "Fiting peak " << LitEnergy[i].at(k) << " From SOURCE " << Source[i] << " Detector " << DetNum << endl;
		int fit_counter = 0;			
		good_fit = true;
		cent = LitEnergy[i].at(k) / cal_pars[j].at(1);
		low = cent - 10;
		upp = cent + 10;
		new_low = low;
		new_upp = upp;
		auto [centr, err, width, chi2, pf_curr, P1_curr] = FitResult(hist[i][j],cent,new_low,new_upp);
		pf[i][j][k] = pf_curr;
		P1[i][j][k] = P1_curr;
		cent = centr;
		while( err == 0 || width > 3. || width < 0.5 || chi2 > 1E5 || cent < new_low || cent > new_upp ){
			cent = LitEnergy[i].at(k) / cal_pars[j].at(1);
			new_low = low + 8*(rand_par.Rndm()-0.5);
			new_upp = upp + 8*(rand_par.Rndm()-0.5);
			auto [centr, err, width, chi2, pf_curr, P1_curr] = FitResult(hist[i][j],cent,new_low,new_upp);
			pf[i][j][k] = pf_curr;
			P1[i][j][k] = P1_curr;
			cent = centr;
			fit_counter++;
			if( fit_counter == 2 ){// if no good fit is found after 10 cycles, move on
				cout << "No good fit found...skipping data point" << endl;
				good_fit = false;
				break;
			}
		}			
		if( good_fit == true ){ //set fit if good fit is found
			cent = centr;
			P1[i][j][k]->GetFitFunction()->SetLineColor(kRed);
			//source_num[DetNum-1].push_back(i); is this line doing nothing?
			gData[i][j]->SetPoint(gData[i][j]->GetN(),cent, LitEnergy[i].at(k) );
			gData[i][j]->SetPointError(gData[i][j]->GetN()-1,err, 0.1 );
			fitnum_counter++;
		}
	}
	for(int k = 0; k < LitEnergy[i].size(); k++){ 
		pf[i][j][k]->Draw("same"); //Draws all fits onto histogram
		P1[i][j][k]->Draw("same");
	}
	hist[i][j]->Write(hist[i][j]->GetName(),TObject::kOverwrite);
	GraphsFile->cd(); gData[i][j]->Write(gData[i][j]->GetName(),TObject::kOverwrite);
	SpectraFile->cd();		
	//C->Close();

}


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

void llt_start_gate(double low, double high, int LaBrNBins = 5000, double LaBrfirstbin = 0.5, double LaBrlastbin = 5000.5, int TacNBins = 50000, double Tacfirstbin = 0, double Taclastbin = 50000){

	outfile->cd();
	llt.Draw(Form("%s:%s>>latac_gate_start_la_%d_%d(%d,%f,%f,%d,%f,%f)",stop.c_str(),tac.c_str(),(int)low,(int)high,TacNBins,Tacfirstbin,Taclastbin,LaBrNBins,LaBrfirstbin,LaBrlastbin),
		Form("%s>%f&&%s<%f",start.c_str(),low,start.c_str(),high ) );
	TH2D *latacdelmat = (TH2D*)outfile->Get(Form("latac_gate_start_la_%f_%f",low,high));
}

void llt_stop_gate(double low, double high, int LaBrNBins = 5000, double LaBrfirstbin = 0.5, double LaBrlastbin = 5000.5, int TacNBins = 50000, double Tacfirstbin = 0, double Taclastbin = 50000){

	outfile->cd();
	llt.Draw(Form("%s:%s>>latac_gate_start_la_%d_%d(%d,%f,%f,%d,%f,%f)",start.c_str(),tac.c_str(),(int)low,(int)high,TacNBins,Tacfirstbin,Taclastbin,LaBrNBins,LaBrfirstbin,LaBrlastbin),
		Form("%s>%f&&%s<%f",stop.c_str(),low,stop.c_str(),high ) );
	TH2D *latacmat = (TH2D*)outfile->Get(Form("latac_gate_stop_la_%f_%f",low,high));
}
