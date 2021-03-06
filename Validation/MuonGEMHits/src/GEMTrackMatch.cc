#include "Validation/MuonGEMHits/interface/GEMTrackMatch.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"


GEMTrackMatch::GEMTrackMatch(const edm::ParameterSet& ps)
{
  minPt_  = ps.getUntrackedParameter<double>("gemMinPt",5.0);
  minEta_ = ps.getUntrackedParameter<double>("gemMinEta",1.55);
  maxEta_ = ps.getUntrackedParameter<double>("gemMaxEta",2.45);
}


GEMTrackMatch::~GEMTrackMatch() {
}

void GEMTrackMatch::FillWithTrigger( MonitorElement* hist[3],Float_t eta)
{
  for( unsigned int i=0 ; i<nstation ; i++) {
    hist[i]->Fill(eta);
  }
  return;
}

void GEMTrackMatch::FillWithTrigger( MonitorElement* hist[3][3],Float_t eta, Float_t phi, bool odd[3], bool even[3])
{
  for( unsigned int i=0 ; i<nstation ; i++) {
    int station = i+1;
    if ( odd[i] && eta > getEtaRange(station,1).first&& eta < getEtaRange(station,1).second ) {
      hist[i][0]->Fill(phi);
      if( hist[i][1] != nullptr) hist[i][1]->Fill(phi);
    }
    if ( even[i] && eta > getEtaRange(station,2).first&& eta < getEtaRange(station,2).second ) {
      hist[i][0]->Fill(phi);
      if( hist[i][2] != nullptr) hist[i][2]->Fill(phi);
    }
  }
  return;
}

void GEMTrackMatch::FillWithTrigger( MonitorElement* hist[4][3], bool array[3][2], Float_t value)
{
  for( unsigned int i=0 ; i<nstation ; i++) {
    if ( array[i][0] ) hist[0][i]->Fill(value);
    if ( array[i][1] ) hist[1][i]->Fill(value);
    if ( array[i][0] || array[i][1] ) hist[2][i]->Fill(value);
    if ( array[i][0] && array[i][1] ) hist[3][i]->Fill(value);
  } 
  return;
}

void GEMTrackMatch::FillWithTrigger( MonitorElement* hist[4][3][3], bool array[3][2], Float_t eta, Float_t phi, bool odd[3], bool even[3])
{
  for( unsigned int i=0 ; i<nstation ; i++) {
    int station = i+1;
    if ( odd[i] && eta > getEtaRange(station,1).first&& eta < getEtaRange(station,1).second ) {
      if ( array[i][0] ) { 
        hist[0][i][0]->Fill(phi);
        if ( hist[0][i][1] != nullptr) hist[0][i][1]->Fill(phi); 
      }
      if ( array[i][1] ) { 
        hist[1][i][0]->Fill(phi); 
        if( hist[1][i][1] != nullptr ) hist[1][i][1]->Fill(phi); 
      }
      if ( array[i][0] || array[i][1] ) { 
        hist[2][i][0]->Fill(phi); 
        if ( hist[2][i][1] != nullptr) hist[2][i][1]->Fill(phi); 
      }
      if ( array[i][0] && array[i][1] ) { 
        hist[3][i][0]->Fill(phi); 
        if ( hist[3][i][1] != nullptr) hist[3][i][1]->Fill(phi); 
      }
    }
    if ( even[i] && eta > getEtaRange(station,2).first&& eta < getEtaRange(station,2).second ) {
      if ( array[i][0] ) { 
        hist[0][i][0]->Fill(phi); 
        if ( hist[0][i][2]!=nullptr) hist[0][i][2]->Fill(phi); 
      }
      if ( array[i][1] ) { 
        hist[1][i][0]->Fill(phi); 
        if( hist[1][i][2]!=nullptr ) hist[1][i][2]->Fill(phi); 
      }
      if ( array[i][0] || array[i][1] ) { 
        hist[2][i][0]->Fill(phi); 
        if( hist[2][i][2]!=nullptr) hist[2][i][2]->Fill(phi); 
      }
      if ( array[i][0] && array[i][1] ) { 
        hist[3][i][0]->Fill(phi); 
        if( hist[3][i][2]!=nullptr) hist[3][i][2]->Fill(phi);
      }
    }
  }
  return;
}

std::pair<double,double> GEMTrackMatch::getEtaRange( int station, int chamber )
{
  if( gem_geom_ != nullptr) {
    auto& ch = gem_geom_->chambers().front();
    auto& roll1 = ch->etaPartitions().front(); //.begin();
    auto& roll2 = ch->etaPartitions()[ch->nEtaPartitions()-1];
    const BoundPlane& bSurface1(roll1->surface());
    const BoundPlane& bSurface2(roll2->surface());
    auto& parameters1( roll1->specs()->parameters());
    float height1(parameters1[2]);
    auto& parameters2( roll2->specs()->parameters());
    float height2(parameters2[2]);
    LocalPoint lTop1( 0., height1, 0.);
    GlobalPoint gTop1(bSurface1.toGlobal(lTop1));
    //LocalPoint lBottom1( 0., -height1, 0.);
    //GlobalPoint gBottom1(bSurface1.toGlobal(lBottom1));
    //LocalPoint lTop2( 0., height2, 0.);
    //GlobalPoint gTop2(bSurface2.toGlobal(lTop2));
    LocalPoint lBottom2( 0., -height2, 0.);
    GlobalPoint gBottom2(bSurface2.toGlobal(lBottom2));
    double eta1 = fabs(gTop1.eta()) - 0.01;
    double eta2 = fabs(gBottom2.eta()) + 0.01;
    return std::make_pair(eta1,eta2);
  }
  else { std::cout<<"Failed to get geometry information"<<std::endl;
    return std::make_pair(0,0);
  }
}


bool GEMTrackMatch::isSimTrackGood(const SimTrack &t)
{

  // SimTrack selection
  if (t.noVertex())   return false; 
  if (t.noGenpart()) return false;
  if (std::abs(t.type()) != 13) return false; // only interested in direct muon simtracks
  if (t.momentum().pt() < minPt_ ) return false;
  const float eta(std::abs(t.momentum().eta()));
  if (eta > maxEta_ || eta < minEta_ ) return false; // no GEMs could be in such eta
  return true;
}


void GEMTrackMatch::buildLUT(const int maxChamberId)
{
  edm::LogInfo("GEMTrackMatch")<<"GEMTrackMatch::buildLUT"<<std::endl;
  edm::LogInfo("GEMTrackMatch")<<"max chamber "<<maxChamberId<<"\n";

  std::vector<int> pos_ids, neg_ids;
  std::vector<float> phis_pos;
  std::vector<float> phis_neg;
  LocalPoint lCentre( 0., 0., 0. );

  for(auto it : gem_geom_->chambers()) {
    if(it->id().region()>0) {
      pos_ids.push_back(it->id().rawId());
      edm::LogInfo("GEMTrackMatch")<<"added id = "<<it->id()<<" = "<<it->id().rawId()<<" to pos ids"<<std::endl;
      const BoundPlane& bSurface(it->surface());
      GlobalPoint gCentre(bSurface.toGlobal(lCentre));
      int cphi(gCentre.phi().degrees());
      if (cphi < 0) cphi += 360;
      phis_pos.push_back(cphi);
      edm::LogInfo("GEMTrackMatch")<<"added phi = "<<cphi<<" to phi pos vector"<<std::endl;
    }
    else if(it->id().region()<0) {
      neg_ids.push_back(it->id().rawId());
      edm::LogInfo("GEMTrackMatch")<<"added id = "<<it->id()<<" = "<<it->id().rawId()<<" to neg ids"<<std::endl;
      const BoundPlane& bSurface(it->surface());
      GlobalPoint gCentre(bSurface.toGlobal(lCentre));
      int cphi(gCentre.phi().degrees());
      if (cphi < 0) cphi += 360;
      phis_neg.push_back(cphi);
      edm::LogInfo("GEMTrackMatch")<<"added phi = "<<cphi<<" to phi neg vector"<<std::endl;
    }
  }
  positiveLUT_ = std::make_pair(phis_pos,pos_ids);
  negativeLUT_ = std::make_pair(phis_neg,neg_ids);
}


void GEMTrackMatch::setGeometry(const GEMGeometry& geom)
{
  edm::LogInfo("GEMTrackMatch")<<"GEMTrackMatch :: setGeometry"<<std::endl;
  gem_geom_ = &geom;
  GEMDetId chId(gem_geom_->chambers().front()->id());
  useRoll_ = 1;
  const auto top_chamber = static_cast<const GEMEtaPartition*>(geom.idToDetUnit(GEMDetId(chId.region(),chId.ring(),chId.station(),chId.layer(),chId.chamber(),useRoll_)));
  const int nEtaPartitions(gem_geom_->chambers().front()->nEtaPartitions());
  const auto bottom_chamber = static_cast<const GEMEtaPartition*>(geom.idToDetUnit(GEMDetId(chId.region(),chId.ring(),chId.station(),chId.layer(),chId.chamber(),nEtaPartitions)));
  const float top_half_striplength = top_chamber->specs()->specificTopology().stripLength()/2.;
  const float bottom_half_striplength = bottom_chamber->specs()->specificTopology().stripLength()/2.;
  const LocalPoint lp_top(0., top_half_striplength, 0.);
  const LocalPoint lp_bottom(0., -bottom_half_striplength, 0.);
  const GlobalPoint gp_top = top_chamber->toGlobal(lp_top);
  const GlobalPoint gp_bottom = bottom_chamber->toGlobal(lp_bottom);

  radiusCenter_ = (gp_bottom.perp() + gp_top.perp())/2.;
  chamberHeight_ = gp_top.perp() - gp_bottom.perp();

  const int maxChamberId = geom.regions()[0]->stations()[0]->superChambers().size();
  edm::LogInfo("GEMTrackMatch")<<"GEMTrackMatch :: setGeometry --> Calling buildLUT"<<std::endl;
  buildLUT(maxChamberId);
}  


std::pair<int,int> GEMTrackMatch::getClosestChambers(const int maxChamberId , int region, float phi)
{
  auto& phis(positiveLUT_.first);
  auto upper = std::upper_bound(phis.begin(), phis.end(), phi);
  auto& LUT = (region == 1 ? positiveLUT_.second : negativeLUT_.second);
  return std::make_pair(LUT.at(upper - phis.begin()), (LUT.at((upper - phis.begin() + 1)%maxChamberId)));
}

std::pair<double, double> GEMTrackMatch::getEtaRangeForPhi( int station ) 
{
  std::pair<double, double> range;
  if( station== 0 )      range = std::make_pair( etaRangeForPhi[0],etaRangeForPhi[1]) ; 
  else if( station== 1 ) range = std::make_pair( etaRangeForPhi[2],etaRangeForPhi[3]) ; 
  else if( station== 2 ) range = std::make_pair( etaRangeForPhi[4],etaRangeForPhi[5]) ; 

  return range;
}

