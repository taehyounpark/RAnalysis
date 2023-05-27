#include <memory>
#include <chrono>
#include <iostream>
#include <sstream>

#include <ROOT/RVec.hxx>
#include "TPad.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTreeReaderValue.h"
#include "TTreeReader.h"
#include "TVector2.h"
#include "TH1F.h"

#include "ana/abc.h"
#include "ana/analysis.h"

#include "rootana/Event.h"
#include "rootana/Hist.h"
#include "rootana/Folder.h"

#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>

using VecF = ROOT::RVec<float>;
using VecD = ROOT::RVec<double>;
using TLV = TLorentzVector;

using cut = ana::selection::cut;
using weight = ana::selection::weight;

int main() {

  auto ds = ana::analysis<Event>({
    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p5169/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_p5169/DAOD_PHYS.29445530._000001.pool.root.1",
  },"CollectionTree");

  auto allMuons = ds.read<xAOD::MuonContainer>("Muons");
  auto eventInfo = ds.read<xAOD::EventInfo>("EventInfo");

  class MuonSelection : public ana::column::definition<ConstDataVector<xAOD::MuonContainer>(xAOD::MuonContainer)>
  {
  public:
    MuonSelection(double etaMax) : m_etaMax(etaMax) {}
    virtual ~MuonSelection() = default;
    virtual ConstDataVector<xAOD::MuonContainer> evaluate( ana::observable<xAOD::MuonContainer> muons ) const override {
      ConstDataVector<xAOD::MuonContainer> selMuons( SG::VIEW_ELEMENTS );
      for( const xAOD::Muon* mu : *muons ) {
        if( TMath::Abs( mu->eta() ) < 1.5 ) {
          selMuons.push_back( mu );
        }
      }
      return selMuons;
    }
  protected:
    double m_etaMax;
  };

  auto selMuons = ds.define<MuonSelection>(1.5)(allMuons);
  auto selMuonsPtMeV = ds.define(
    [](ConstDataVector<xAOD::MuonContainer> const& muons){
      VecD pts;
      for( const xAOD::Muon* mu : muons ) {
        pts.push_back(mu->pt());
      }
    return pts;
    })
    (selMuons);
  auto selMuonsPt = selMuonsPtMeV / ds.constant(1000.);

  auto mcEventWeight = ds.define([](const xAOD::EventInfo& eventInfo){return eventInfo.mcEventWeight();})(eventInfo);
  auto mcEventWeighted = ds.filter<weight>("mcEventWeight")(mcEventWeight);

  auto selMuonsPtHist = ds.book<Hist<1,VecF>>("muons_pt",100,0,100).fill(selMuonsPt).at(mcEventWeighted);

  selMuonsPtHist->Draw();
  gPad->Print("muons_pt.pdf");

  return 0;
}