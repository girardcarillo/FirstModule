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

// Macro which automatically implements the interface needed
// to enable the module to be loaded at runtime
// The first argument is the typename
// The second is the string key used to access the module in pipeline
// scripts. This must be globally unique.
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
  _sd_tree_->Branch("energy", &_energy_,"energy/D");
  _sd_tree_->Branch("time", &_time_,"time/D");

  this->_set_initialized(true);
}

// Process
dpp::base_module::process_status
MySecondModule::process(datatools::things& data_record_) {
  DT_THROW_IF(! is_initialized(), std::logic_error,
              "Module '" << get_name () << "' is not initialized !");

  //topology data (storing the calibrated datas)
  const std::string & sd_label = snemo::datamodel::data_info::default_simulated_data_label();

  // Counting the total number of event for the simulation
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
    //double the_total_energy = 0;
    double the_time_difference = 0;
    int the_number_of_simulated_electrons = 0;
    for (const auto & iparticle : the_primary_particles) {
      // if (iparticle.is_gamma()) {
      //   const double a_time = iparticle.get_time();
      //   const double a_energy = iparticle.get_total_energy();
      //   DT_LOG_NOTICE(get_logging_priority(),
      //                 "Particle is a gamma with energy = " << a_energy/CLHEP::keV << " keV "
      //                 << "and time = " << a_time/CLHEP::picosecond << "ps");
      // }
      if (iparticle.is_electron()) {
        const double a_time = iparticle.get_time();
        //const double a_energy = iparticle.get_total_energy();
        //const double a_mass = iparticle.get_mass();
        //const double a_kinetic_energy = iparticle.get_kinetic_energy();
        // DT_LOG_NOTICE(get_logging_priority(),
        //               "Particle is an electron with energy = " << a_energy/CLHEP::keV << " keV "
        //               << " and time = " << a_time/CLHEP::picosecond << " ps "
        //               /*<<",mass = "<<a_mass/CLHEP::keV<<" keV "
        //                 <<"and kinetic energy = "<<a_kinetic_energy/CLHEP::keV<<" keV "*/);
        // the_total_energy = the_total_energy + a_energy;
        the_time_difference = fabs(the_time_difference - a_time);
        the_number_of_simulated_electrons++;
      }
    }
      // _sd_output_file_->cd();
      // _time_=the_time_difference;
      // _sd_tree_->Fill();

    //checking if TD has 2e pattern
    if (data_record_.has("TD")) {
      const snemo::datamodel::topology_data & a_td
        =  data_record_.get<snemo::datamodel::topology_data>("TD");
      if (a_td.has_pattern()
          && a_td.has_pattern_as<snemo::datamodel::topology_2e_pattern>()
          && the_number_of_simulated_electrons == 2) {
        DT_LOG_DEBUG(get_logging_priority(), "The TD has a 2e pattern");
        const snemo::datamodel::topology_2e_pattern & a_2e_topology
          = a_td.get_pattern_as<snemo::datamodel::topology_2e_pattern>();
        const double & a_energy_sum
          =a_2e_topology.get_electrons_energy_sum();

        if (a_energy_sum/CLHEP::MeV > 2.7
            && a_energy_sum/CLHEP::MeV < 3.2) {
          //Fill the root file
          _sd_output_file_->cd();
          //_energy_=a_energy_sum/CLHEP::keV;//Fill a root file with the TD total energy for a 2e pattern
          _time_=the_time_difference/CLHEP::picosecond;//Fill a root file with the SD times for a 2e pattern
          _sd_tree_->Fill();
          _number_of_events_with_energy_cut_++;
        }
      }
    }
    // if (the_total_energy/CLHEP::MeV > 2.7
    //     && the_total_energy/CLHEP::MeV < 3.2
    //     && the_number_of_simulated_electrons == 2) {

    //   DT_LOG_NOTICE(get_logging_priority(),
    //                 "Primary event with two electrons with total energy = " << the_total_energy/CLHEP::keV << " keV "
    //                 << " and time difference = " << the_time_difference/CLHEP::picosecond << " ps ");

    //   _sd_output_file_->cd();
    //   _time_=the_time_difference/CLHEP::picosecond;
    //   _energy_=the_total_energy/CLHEP::keV;
    //   _sd_tree_->Fill();
    // }
  }
  double rate_of_events_with_energy_cut = (_number_of_events_with_energy_cut_/_number_of_events_)*100;
  std::cout<<"The rate of events with energy cut = "<<rate_of_events_with_energy_cut<<" %"<<std::endl;
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
