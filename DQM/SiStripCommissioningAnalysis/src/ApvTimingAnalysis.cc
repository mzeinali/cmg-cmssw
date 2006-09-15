#include "DQM/SiStripCommissioningAnalysis/interface/ApvTimingAnalysis.h"
#include "DQM/SiStripCommon/interface/SiStripHistoNamingScheme.h"
#include "TProfile.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

// ----------------------------------------------------------------------------
// 
ApvTimingAnalysis::ApvTimingAnalysis( const uint32_t& key ) 
  : CommissioningAnalysis(key),
    time_(sistrip::invalid_), 
    maxTime_(sistrip::invalid_), 
    delay_(sistrip::invalid_), 
    error_(sistrip::invalid_), 
    base_(sistrip::invalid_), 
    peak_(sistrip::invalid_), 
    height_(sistrip::invalid_),
    histo_(0,""),
    optimumSamplingPoint_(15.)
{;}
// ----------------------------------------------------------------------------
// 
ApvTimingAnalysis::ApvTimingAnalysis() 
  : CommissioningAnalysis(),
    time_(sistrip::invalid_), 
    maxTime_(sistrip::invalid_), 
    delay_(sistrip::invalid_), 
    error_(sistrip::invalid_), 
    base_(sistrip::invalid_), 
    peak_(sistrip::invalid_), 
    height_(sistrip::invalid_),
    histo_(0,""),
    optimumSamplingPoint_(15.)
{;}

// ----------------------------------------------------------------------------
// 
void ApvTimingAnalysis::print( stringstream& ss, uint32_t not_used ) { 
  if ( key() ) {
    ss << "APV TIMING monitorables for channel key 0x"
       << hex << setw(8) << setfill('0') << key() << dec << "\n";
  } else {
    ss << "APV TIMING monitorables" << "\n";
  }
  float adjust = sistrip::invalid_;
  if ( time_ < sistrip::maximum_ && 
       delay_ < sistrip::maximum_ ) { adjust = time_ + delay_; }
  ss << " Time of tick mark rising edge [ns]        : " << time_ << "\n" 
     << " Error on time of rising edge [ns]         : " << error_ << "\n"
     << " Sampling point of last tick mark [ns]     : " << maxTime_ + optimumSamplingPoint_ << "\n" 
     << " Adjusted sampling point of last tick [ns] : " << adjust << "\n" 
     << " Delay required to synchronise [ns]        : " << delay_ << "\n" 
     << " Baseline level [adc]                      : " << base_ << "\n" 
     << " Tick peak level [adc]                     : " << peak_ << "\n" 
     << " Tick mark height [adc]                    : " << height_ << "\n";
}

// ----------------------------------------------------------------------------
// 
void ApvTimingAnalysis::reset() {
  time_ = sistrip::invalid_; 
  maxTime_ = sistrip::invalid_; 
  delay_ = sistrip::invalid_; 
  error_ = sistrip::invalid_; 
  base_ = sistrip::invalid_; 
  peak_ = sistrip::invalid_; 
  height_ = sistrip::invalid_;
  histo_ = Histo(0,"");
}

// ----------------------------------------------------------------------------
// 1) Calculates position of "sampling point" of last tick; 2) then
// adjusts so that it is multiple of 25, ie, synched with FED
// sampling; 3) then calculates delay required to synchronise with
// this adjusted sampling position.
void ApvTimingAnalysis::maxTime( const float& time ) { 
  maxTime_ = time;
  if ( time_ > sistrip::maximum_ ) { return; }
  float adjustment = 25 - static_cast<int32_t>( rint(maxTime_+optimumSamplingPoint_) ) % 25;
  delay_ = ( maxTime_ + adjustment ) - time_; 
}

// ----------------------------------------------------------------------------
// 
void ApvTimingAnalysis::extract( const vector<TProfile*>& histos ) { 
  
  // Check
  if ( histos.size() != 1 ) {
    cerr << "[" << __PRETTY_FUNCTION__ << "]"
	 << " Unexpected number of histograms: " 
	 << histos.size()
	 << endl;
  }
  
  // Extract
  vector<TProfile*>::const_iterator ihis = histos.begin();
  for ( ; ihis != histos.end(); ihis++ ) {
    
    // Check pointer
    if ( !(*ihis) ) {
      cerr << "[" << __PRETTY_FUNCTION__ << "]"
	   << " NULL pointer to histogram!" << endl;
      continue;
    }
    
    // Check name
    static SiStripHistoNamingScheme::HistoTitle title;
    title = SiStripHistoNamingScheme::histoTitle( (*ihis)->GetName() );
    if ( title.task_ != sistrip::APV_TIMING ) {
      cerr << "[" << __PRETTY_FUNCTION__ << "]"
	   << " Unexpected commissioning task!"
	   << "(" << SiStripHistoNamingScheme::task( title.task_ ) << ")"
	   << endl;
      continue;
    }
    
    // Extract timing histo
    histo_.first = *ihis;
    histo_.second = (*ihis)->GetName();
    
  }
  
}

// ----------------------------------------------------------------------------
// 
void ApvTimingAnalysis::analyse() { 
  if ( !histo_.first ) {
    cerr << "[" << __PRETTY_FUNCTION__ << "]"
	 << " NULL pointer to histogram!" << endl;
    return;
  }
  
  // Transfer histogram contents/errors/stats to containers
  uint16_t non_zero = 0;
  float max = -1.e9;
  float min =  1.e9;
  uint16_t nbins = static_cast<uint16_t>( histo_.first->GetNbinsX() );
  vector<float> bin_contents; 
  vector<float> bin_errors;
  vector<float> bin_entries;
  bin_contents.reserve( nbins );
  bin_errors.reserve( nbins );
  bin_entries.reserve( nbins );
  for ( uint16_t ibin = 0; ibin < nbins; ibin++ ) {
    bin_contents.push_back( histo_.first->GetBinContent(ibin+1) );
    bin_errors.push_back( histo_.first->GetBinError(ibin+1) );
    bin_entries.push_back( histo_.first->GetBinEntries(ibin+1) );
    if ( bin_entries[ibin] ) { 
      if ( bin_contents[ibin] > max ) { max = bin_contents[ibin]; }
      if ( bin_contents[ibin] < min ) { min = bin_contents[ibin]; }
      non_zero++;
    }
  }
  //cout << " Number of bins with non-zero entries: " << non_zero << endl;
  if ( bin_contents.size() < 100 ) { 
    cerr << "[" << __PRETTY_FUNCTION__ << "]"
 	 << " Too few bins! Number of bins: " 
 	 << bin_contents.size() << endl;
    return; 
  }
  
  // Calculate range (max-min) and threshold level (range/2)
  float range = max - min;
  float threshold = min + range / 2.;
  if ( range < 50. ) {
    cerr << "[" << __PRETTY_FUNCTION__ << "]"
 	 << " Signal range (max - min) is too small: " << range << endl;
    return; 
  }
  //cout << " ADC samples: max/min/range/threshold: " 
  //<< max << "/" << min << "/" << range << "/" << threshold << endl;
  
  // Associate samples with either "tick mark" or "baseline"
  vector<float> tick;
  vector<float> base;
  for ( uint16_t ibin = 0; ibin < nbins; ibin++ ) { 
    if ( bin_entries[ibin] ) {
      if ( bin_contents[ibin] < threshold ) { 
	base.push_back( bin_contents[ibin] ); 
      } else { 
	tick.push_back( bin_contents[ibin] ); 
      }
    }
  }
  //cout << " Number of 'tick mark' samples: " << tick.size() 
  //<< " Number of 'baseline' samples: " << base.size() << endl;
  
  // Find median level of tick mark and baseline
  float tickmark = 0.;
  float baseline = 0.;
  sort( tick.begin(), tick.end() );
  sort( base.begin(), base.end() );
  if ( !tick.empty() ) { tickmark = tick[ tick.size()%2 ? tick.size()/2 : tick.size()/2 ]; }
  if ( !base.empty() ) { baseline = base[ base.size()%2 ? base.size()/2 : base.size()/2 ]; }
  //cout << " Tick mark level: " << tickmark << " Baseline level: " << baseline
  //<< " Range: " << (tickmark-baseline) << endl;
  if ( (tickmark-baseline) < 50. ) {
    cerr << "[" << __PRETTY_FUNCTION__ << "]"
 	 << " Range b/w tick mark height ("  << tickmark
	 << ") and baseline ("  << baseline
	 << ") is too small ("  << (tickmark-baseline)
	 << ")." << endl;
    return; 
  }
  
  // Find rms spread in "baseline" samples
  float mean = 0.;
  float mean2 = 0.;
  for ( uint16_t ibin = 0; ibin < base.size(); ibin++ ) {
    mean += base[ibin];
    mean2 += base[ibin] * base[ibin];
  }
  if ( !base.empty() ) { 
    mean = mean / base.size();
    mean2 = mean2 / base.size();
  } else { 
    mean = 0.; 
    mean2 = 0.; 
  }
  float baseline_rms = 0.;
  if (  mean2 > mean*mean ) { baseline_rms = sqrt( mean2 - mean*mean ); }
  else { baseline_rms = 0.; }
  //cout << " Spread in baseline samples: " << baseline_rms << endl;
  
  // Find rising edges (derivative across two bins > range/2) 
  map<uint16_t,float> edges;
  for ( uint16_t ibin = 1; ibin < nbins-1; ibin++ ) {
    if ( bin_entries[ibin+1] && 
	 bin_entries[ibin-1] ) {
      float derivative = bin_contents[ibin+1] - bin_contents[ibin-1];
      if ( derivative > 5.*baseline_rms ) {
	edges[ibin] = derivative;
	//cout << " Found edge #" << edges.size() << " at bin " << ibin 
	//<< " and with derivative " << derivative << endl;
      }
    }
  }
  
  // Iterate through "edges" map
  bool found = false;
  uint16_t deriv_bin = sistrip::invalid_;
  float max_deriv = -1.*sistrip::invalid_;
  map<uint16_t,float>::iterator iter = edges.begin();
  while ( !found && iter != edges.end() ) {

    // Iterate through 50 subsequent samples
    bool valid = true;
    for ( uint16_t ii = 0; ii < 50; ii++ ) {
      uint16_t bin = iter->first + ii;

      // Calc local derivative 
      float temp_deriv = 0;
      if ( static_cast<uint32_t>(bin-1) < 0 ||
	   static_cast<uint32_t>(bin+1) >= nbins ) { continue; }
      temp_deriv = bin_contents[bin+1] - bin_contents[bin-1];
      
      // Store max derivative
      if ( temp_deriv > max_deriv ) {
	max_deriv = temp_deriv;
	deriv_bin = bin;
      }

      // Check if samples following edge are all "high"
      if ( ii > 10 && ii < 40 && bin_entries[bin] &&
	   bin_contents[bin] < baseline + 5*baseline_rms ) { valid = false; }

    }

    // Break from loop if tick mark found
    if ( valid ) { found = true; }
    else {
      max_deriv = -1.*sistrip::invalid_;
      deriv_bin = sistrip::invalid_;
      edges.erase(iter);
    }

    iter++;
  }

  //   cout << " Identified " << edges.size() << " edges followed by tick! #/bin/derivative: ";
  //   uint16_t cntr = 0;
  //   for ( map<uint16_t,float>::const_iterator iter = edges.begin();
  // 	iter != edges.end(); iter++ ) {
  //     cout << cntr++ << "/" << iter->first << "/" << iter->second << ", ";
  //   }
  //   cout << endl; 
  
  // Set monitorables
  if ( deriv_bin < sistrip::maximum_ ) {
    time_      = deriv_bin * 25. / 24.;
    error_     = 0.;
    base_      = baseline;
    peak_      = tickmark;
    height_    = tickmark - baseline;
  } else {
    cerr << "[" << __PRETTY_FUNCTION__ << "]"
	 << " No tick marks found!" << endl;
    base_   = baseline;
    peak_   = tickmark;
    height_ = tickmark - baseline;
  }
  
}
