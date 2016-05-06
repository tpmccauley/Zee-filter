import FWCore.ParameterSet.Config as cms

ZeeFilter = cms.EDFilter('ZeeFilter',
                           electronInputTag = cms.InputTag('gsfElectrons'),
                           csvFileName = cms.string('Zee.csv'),
                           invariantMassMin = cms.double(60.0),
                           invariantMassMax = cms.double(120.0)
                           )
