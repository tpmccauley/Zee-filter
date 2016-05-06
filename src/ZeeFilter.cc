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
  double last_pt = 0.0;
  double last_eta = 0.0;
  double last_phi = 0.0;

  // NEEDED: selection cuts, i.e. isolation, etc.

  for ( reco::GsfElectronCollection::const_iterator ei = electrons->begin(), eie = electrons->end();
        ei != eie; ++ei )
  {   
      pt = ei->pt();
      eta = ei->eta();
      phi = ei->phi();
      charge = ei->charge();

      if ( last_charge == 0 ) // i.e. first of a potential pair
      {
        last_charge = charge;
        last_pt = pt;
        last_eta = eta;
        last_phi = phi;
      }
      
      else 
      {
        if ( charge == last_charge )
          return false;
        
        double M = 2*last_pt*pt*(cosh(last_eta-eta)-cos(last_phi-phi));
        M = sqrt(M);

        if ( M < invariantMassMin_ || M > invariantMassMax_ )
          return false;

        csvOut_<< iEvent.id().run() <<","<< iEvent.id().event() <<","
               << last_pt <<","<< last_eta <<","<< last_phi <<","<< last_charge <<","
               << pt <<","<< eta <<","<< phi <<","<< charge <<","
               << M <<std::endl;
      }
  }

  return true;
}

void 
ZeeFilter::beginJob()
{
  csvOut_<<"Run,Event,pt1,eta1,phi1,Q1,pt2,eta2,phi2,Q2,M"<<std::endl;
}

void 
ZeeFilter::endJob() 
{
  csvOut_.close();
}

DEFINE_FWK_MODULE(ZeeFilter);
