// - Implementation of MySecondModule
//
#include "MySecondModule.h"
// Standard Library
// Third Party
// - A
// This Project
#include <snemo/datamodels/data_model.h>
#include <mctools/simulated_data.h>

#include <falaise/snemo/datamodels/pid_utils.h>
#include <snemo/datamodels/particle_track_data.h>
#include <snemo/datamodels/calibrated_data.h>
#include <snemo/datamodels/calibrated_calorimeter_hit.h>
#include <snemo/datamodels/topology_data.h>

#include <snemo/datamodels/topology_exporter.h>
#include <snemo/datamodels/export_topology.h>
#include <snemo/datamodels/base_data_bank_exporter.h>

#include <snemo/datamodels/topology_2e_pattern.h>

#include <snemo/datamodels/particle_track.h>

#include <fstream>

// function definition
double g_EnergyCut (double energy_sum, double energy_max, double energy_min) {
  if (energy_sum/CLHEP::MeV > energy_min
      && energy_sum/CLHEP::MeV < energy_max) {
    _number_of_internal_conversion_events_++;
    std::cout<<"number of internal conversion events"<<_number_of_internal_conversion_events_<<std::endl;
    if (_the_time_difference_ != 0) {
      _sd_output_file_->cd();
      _time_= _the_time_difference_/CLHEP::picosecond;//Fill a root file with the SD times for a 2e pattern
      _sd_tree_->Fill();
      _on_exponential_++;
    }
  }
}

DPP_MODULE_REGISTRATION_IMPLEMENT(MySecondModule,"MySecondModule")

// Construct
MySecondModule::MySecondModule() : dpp::base_module()
{
  this->_set_defaults();
}

// Destruct
MySecondModule::~MySecondModule() {
  // MUST reset module at destruction
  this->reset();
}

// Initialize
void MySecondModule::initialize(const datatools::properties& setup_,
                                datatools::service_manager& /*flServices*/,
                                dpp::module_handle_dict_type& /*moduleDict*/) {

  DT_THROW_IF(is_initialized(), std::logic_error,
              "Module '" << get_name () << "' is not initialized !");

  dpp::base_module::_common_initialize(setup_);

  _sd_output_file_ = new TFile ("sd_tree.root", "recreate", "Output file of Simulation data");

  //Root tree
  _sd_output_file_->cd();
  _sd_tree_= new TTree("calorimeter hit tree", "calorimeter hit tree");
  _sd_tree_->Branch("time", &_time_,"time/D");

  this->_set_initialized(true);
}

std::string const final_rate("final_rate.txt");
std::string const other_events("other_events.txt");

std::ofstream final_flux(final_rate.c_str());
std::ofstream other_events_flux(other_events.c_str());

// Process
dpp::base_module::process_status
MySecondModule::process(datatools::things& data_record_) {
  DT_THROW_IF(! is_initialized(), std::logic_error,
              "Module '" << get_name () << "' is not initialized !");

  //topology data (storing the calibrated datas)
  const std::string & sd_label = snemo::datamodel::data_info::default_simulated_data_label();

  // Counting the total number of events for the simulation
  _number_of_events_++;

  // SD bank
  if (data_record_.has(sd_label)) {
    const mctools::simulated_data & a_sd
      =  data_record_.get<mctools::simulated_data>(sd_label);
    const mctools::simulated_data::primary_event_type & a_primary_event
      = a_sd.get_primary_event();
    const genbb::primary_event::particles_col_type & the_primary_particles
      = a_primary_event.get_particles();

    //checking if SD has 2e pattern
    for (const auto & iparticle : the_primary_particles) {
      if (iparticle.is_electron()) {
        const double a_time = iparticle.get_time();
        _the_time_difference_ = fabs(_the_time_difference_ - a_time);
        _the_number_of_simulated_electrons_++;
      }
    }

    //checking if TD has 2e pattern
    if (data_record_.has("TD")) {
      const snemo::datamodel::topology_data & a_td
        =  data_record_.get<snemo::datamodel::topology_data>("TD");
      if (a_td.has_pattern()
          && a_td.has_pattern_as<snemo::datamodel::topology_2e_pattern>()){
        const snemo::datamodel::topology_2e_pattern & a_2e_topology
          = a_td.get_pattern_as<snemo::datamodel::topology_2e_pattern>();
        const double & a_energy_sum
          = a_2e_topology.get_electrons_energy_sum();
        if (_the_number_of_simulated_electrons_ == 2){
          DT_LOG_DEBUG(get_logging_priority(), "The TD has a 2e pattern");

          //Energy cut
          //with g_EnergyCut function
          g_EnergyCut(a_energy_sum, 2.7, 3.2);

          //without g_EnergyCut function
          // if (a_energy_sum/CLHEP::MeV > 2.7
          //     && a_energy_sum/CLHEP::MeV < 3.2) {
          //   _number_of_internal_conversion_events_++;
          //   std::cout<<"number of internal conversion events"<<_number_of_internal_conversion_events_<<std::endl;
          //   if (_the_time_difference_ != 0) {
          //     _sd_output_file_->cd();
          //     _time_= _the_time_difference_/CLHEP::picosecond;//Fill a root file with the SD times for a 2e pattern
          //     _sd_tree_->Fill();
          //     _on_exponential_++;
          //   }
          // }
        }
        else {
          if (a_energy_sum/CLHEP::MeV > 2.7
              && a_energy_sum/CLHEP::MeV < 3.2) {
            _number_of_other_events_++;
            if (other_events_flux) {
              for (const auto & iparticle : the_primary_particles) {
                const std::string & the_particle_label
                  = iparticle.get_particle_label();
                const double a_time = iparticle.get_time();
                other_events_flux<<"Event #"<<_number_of_events_-1<<std::endl;
                other_events_flux<<"Particle type = "<<the_particle_label<<std::endl;
                other_events_flux<<"Event generation time"<<a_time<<std::endl;
              }
            }
            else {
              std::cout<< "ERREUR: Impossible d'ouvrir le fichier other_events.txt" <<std::endl;
            }
          }
        }
      }
    }
  }

  double rate_of_internal_conversion_events = (_number_of_internal_conversion_events_/_number_of_events_)*100;
  double rate_on_exponential = (_on_exponential_/_number_of_events_)*100;
  double rate_of_other_events = (_number_of_other_events_/_number_of_events_)*100;
  double total_rate = ((_number_of_internal_conversion_events_ + _number_of_other_events_)/_number_of_events_)*100;

  if (final_flux) {
    final_flux<<"Event #"<<_number_of_events_-1<<std::endl;
    final_flux<<total_rate<<" % "<<"2e pattern, of which :"<<std::endl;
    final_flux<<"  - "<<rate_of_internal_conversion_events<<" % "<<"generated ("<<rate_on_exponential<<" % with non zero time difference),"<<std::endl;
    final_flux<<"  - "<<rate_of_other_events<<" % "<<"other contaminations (Compton, Moller)"<<std::endl;
  }
  else {
    std::cout<< "ERREUR: Impossible d'ouvrir le fichier final_rate.txt"<<std::endl;
  }

  return PROCESS_OK;
}

// Reset
void MySecondModule::reset()
{
  // Root tree
  _sd_output_file_->cd();
  _sd_tree_->Write();
  _sd_output_file_->Close();

  _set_defaults();

  // Tag MySecondModule as un-initialized
  this->_set_initialized(false);
}

//Set defaults values of data members
void MySecondModule::_set_defaults() {
}
