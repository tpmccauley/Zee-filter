import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.ParameterSet.Types as CfgTypes

process = cms.Process("opendata")

goodJSON = 'Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt'
myLumis = LumiList.LumiList(filename = goodJSON).getCMSSWString().split(',')

import FWCore.Utilities.FileUtils as FileUtils
from FWCore.MessageLogger.MessageLogger_cfi import *

doubleElectronFiles = FileUtils.loadListFromFile('CMS_Run2011A_DoubleElectron_AOD_12Oct2013-v1_20000_file_index.txt')
process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring(*doubleElectronFiles)
                            )

process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange()
process.source.lumisToProcess.extend(myLumis)

process.load('ZeeFilter.ZeeFilter.ZeeFilter_cfi')

process.ZeeFilter.csvFileName = cms.string('Zee_Run2011A.csv')
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(500000))

process.mypath = cms.Path(process.ZeeFilter)
process.schedule = cms.Schedule(process.mypath)
