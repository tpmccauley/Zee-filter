//
// Original Author: thomas.mccauley@cern.ch
//

#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrackExtra.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrackFwd.h"

#include <iostream>
#include <string>
#include <fstream>

class ZeeFilter : public edm::EDFilter {
public:
  explicit ZeeFilter(const edm::ParameterSet&);
  ~ZeeFilter();

private:
  virtual void beginJob() ;
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  edm::InputTag electronInputTag_;
  
  std::ofstream csvOut_;
  std::string csvFileName_;
  
  double invariantMassMin_;
  double invariantMassMax_;
};

ZeeFilter::ZeeFilter(const edm::ParameterSet& iConfig)
  : electronInputTag_(iConfig.getParameter<edm::InputTag>("electronInputTag")),
    csvFileName_(iConfig.getParameter<std::string>("csvFileName")),
    invariantMassMin_(iConfig.getParameter<double>("invariantMassMin")),
    invariantMassMax_(iConfig.getParameter<double>("invariantMassMax"))
{
  csvOut_.open(csvFileName_.c_str());
}

ZeeFilter::~ZeeFilter()
{}

bool
ZeeFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<reco::GsfElectronCollection> electrons;
  iEvent.getByLabel(electronInputTag_, electrons);

  if ( ! electrons.isValid() )
  {
    std::cerr<<"ZeeFilter: invalid collection"<<std::endl;
    return false;
  }
  
  // For now, for simplicity's sake, let's only use events where there are only 2 electrons
  if ( electrons->size() != 2 )
    return false;

  int charge;
  double pt, eta, phi;

  int last_charge = 0;
  double last_pt, last_eta, last_phi;

  float sigmaEtaEta, hcalOverEcal, isoTrack, isoECAL, isoHCAL;
  float last_sigmaEtaEta, last_hcalOverEcal, last_isoTrack, last_isoECAL, last_isoHCAL;

  std::string type;
  std::string last_type;

  for ( reco::GsfElectronCollection::const_iterator ei = electrons->begin(), eie = electrons->end();
        ei != eie; ++ei )
  {   
      pt = ei->pt();

      if ( pt < 25 )
        return false;

      eta = ei->eta();
      phi = ei->phi();
      charge = ei->charge();

      sigmaEtaEta = ei->sigmaEtaEta();
      hcalOverEcal = ei->hcalOverEcal();
      
      isoTrack = ei->dr04TkSumPt();
      isoECAL = ei->dr04EcalRecHitSumEt(); 
      isoHCAL = ei->dr04HcalTowerSumEt();

      if ( ei->isEB() ) 
        type = "EB";
        
      if ( ei->isEE() ) 
        type = "EE";

      if ( last_charge == 0 ) // i.e. first of a potential pair
      {
        last_charge = charge;
        last_pt = pt;
        last_eta = eta;
        last_phi = phi;

        last_sigmaEtaEta = sigmaEtaEta;
        last_hcalOverEcal = hcalOverEcal;

        last_isoTrack = isoTrack;
        last_isoECAL = isoECAL;
        last_isoHCAL = isoHCAL;

        last_type = type;
      }
      
      else 
      {
        //if ( charge == last_charge )
        //  return false;
        
        double M = 2*last_pt*pt*(cosh(last_eta-eta)-cos(last_phi-phi));
        M = sqrt(M);

        if ( M < invariantMassMin_ || M > invariantMassMax_ )
          return false;

        csvOut_<< iEvent.id().run() <<","<< iEvent.id().event() <<","
               << last_pt <<","<< last_eta <<","<< last_phi <<","<< last_charge <<","<< last_type <<","
               << last_sigmaEtaEta <<","<< last_hcalOverEcal <<","<< last_isoTrack <<","<< last_isoECAL <<","<< last_isoHCAL <<","
               << pt <<","<< eta <<","<< phi <<","<< charge <<","<< type <<","
               << sigmaEtaEta <<","<< hcalOverEcal <<","<< isoTrack <<","<< isoECAL <<","<< isoHCAL <<std::endl;
      }
  }

  return true;
}

void 
ZeeFilter::beginJob()
{
  csvOut_<<"Run,Event,pt1,eta1,phi1,Q1,type1,sigmaEtaEta1,HoverE1,isoTrack1,isoEcal1,isoHcal1,pt2,eta2,phi2,Q2,type2,sigmaEtaEta2,HoverE2,isoTrack2,isoEcal2,isoHcal2"<<std::endl;
}

void 
ZeeFilter::endJob() 
{
  csvOut_.close();
}

DEFINE_FWK_MODULE(ZeeFilter);
