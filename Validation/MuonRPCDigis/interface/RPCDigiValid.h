#ifndef RPCDigiValid_h
#define RPCDigiValid_h

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include <iostream>
#include <string>
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1F.h"

#include "FWCore/Utilities/interface/EDGetToken.h"
#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

class RPCDigiValid: public edm::EDAnalyzer
{

public:

  RPCDigiValid(const edm::ParameterSet& ps);
  ~RPCDigiValid();

protected:
  void analyze(const edm::Event& e, const edm::EventSetup& c);
  void beginJob();
  void endJob(void);
  void beginRun(edm::Run const&, edm::EventSetup const&);
  void endRun(edm::Run const&, edm::EventSetup const&);

private:

  MonitorElement* xyview;
  MonitorElement* rzview;
  MonitorElement* Res;
  MonitorElement* ResWmin2;
  MonitorElement* ResWmin1;
  MonitorElement* ResWzer0;
  MonitorElement* ResWplu1;
  MonitorElement* ResWplu2;
  MonitorElement* BxDist;
  MonitorElement* StripProf;

  //barrel layers residuals
  MonitorElement* ResLayer1_barrel;
  MonitorElement* ResLayer2_barrel;
  MonitorElement* ResLayer3_barrel;
  MonitorElement* ResLayer4_barrel;
  MonitorElement* ResLayer5_barrel;
  MonitorElement* ResLayer6_barrel;

  //members for EndCap's disks:
  MonitorElement* ResDmin1;
  MonitorElement* ResDmin2;
  MonitorElement* ResDmin3;
  MonitorElement* ResDplu1;
  MonitorElement* ResDplu2;
  MonitorElement* ResDplu3;

  //endcap layters residuals
  MonitorElement* Res_Endcap1_Ring2_A;
  MonitorElement* Res_Endcap1_Ring2_B;
  MonitorElement* Res_Endcap1_Ring2_C;

  MonitorElement* Res_Endcap23_Ring2_A;
  MonitorElement* Res_Endcap23_Ring2_B;
  MonitorElement* Res_Endcap23_Ring2_C;

  MonitorElement* Res_Endcap123_Ring3_A;
  MonitorElement* Res_Endcap123_Ring3_B;
  MonitorElement* Res_Endcap123_Ring3_C;

  //new member for cls
  MonitorElement* noiseCLS;
  MonitorElement* noiseCLSBarrel;
  MonitorElement* noiseCLSEndcaps;

  MonitorElement* clsBarrel;

  //CLS Validation
  //ring2, disk +- 1
  MonitorElement* CLS_Endcap_1_Ring2_A;
  MonitorElement* CLS_Endcap_1_Ring2_B;
  MonitorElement* CLS_Endcap_1_Ring2_C;

  MonitorElement* CLS_Endcap_23_Ring2_A;
  MonitorElement* CLS_Endcap_23_Ring2_B;
  MonitorElement* CLS_Endcap_23_Ring2_C;

  //ring 3
  MonitorElement* CLS_Endcap_123_Ring3_A;
  MonitorElement* CLS_Endcap_123_Ring3_B;
  MonitorElement* CLS_Endcap_123_Ring3_C;
  //CLS Validation

  //4 endcap
  MonitorElement *ResDmin4;
  MonitorElement *ResDplu4;
  MonitorElement *BxDisc_4Plus;
  MonitorElement *BxDisc_4Min;
  MonitorElement *xyvDplu4;
  MonitorElement *xyvDmin4;
  MonitorElement *CLS_Endcap_4;

  DQMStore* dbe_;
  std::string outputFile_;
  std::string digiLabel;

  //Tokens for accessing run data. Used for passing to edm::Event. - stanislav
  edm::EDGetTokenT<edm::PSimHitContainer> simHitToken;
  edm::EDGetTokenT<RPCDigiCollection> rpcDigiToken;
};

#endif

