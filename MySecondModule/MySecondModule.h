//! \file    MySecondModule.h
//! \brief   Example processing module for flreconstruct
//! \details Process a things object
#ifndef MYSECONDMODULE_HH
#define MYSECONDMODULE_HH
// Standard Library
// Third Party
// - Bayeux
#include <bayeux/dpp/base_module.h>

// Root objects
#include <TFile.h>
#include <TTree.h>

#include <falaise/snemo/datamodels/base_topology_pattern.h>


class MySecondModule : public dpp::base_module
{
public:

  //! Construct module
  MySecondModule();

  //! Destructor
  virtual ~MySecondModule();

  //! Configure the module
  virtual void initialize(const datatools::properties& myConfig,
                          datatools::service_manager& flServices,
                          dpp::module_handle_dict_type& moduleDict);

  //! Process supplied data record
  virtual dpp::base_module::process_status process(datatools::things& workItem);

  // //functions
  // double g_EnergyCut (double energy_sum, double energy_max, double energy_min);

  // function definition
  void g_EnergyCut (double energy_sum, double energy_max, double energy_min) {
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

  // void g_ComptonEnergyCut (double energy_sum, double energy_max, double energy_min) {
  //   if (energy_sum/CLHEP::MeV > energy_min
  //       && energy_sum/CLHEP::MeV < energy_max) {
  //     _number_of_other_events_++;
  //     if (other_events_flux) {
  //       for (const auto & iparticle : the_primary_particles) {
  //         const std::string & the_particle_label
  //           = iparticle.get_particle_label();
  //         const double a_time = iparticle.get_time();
  //         other_events_flux<<"Event #"<<_number_of_events_-1<<std::endl;
  //         other_events_flux<<"Particle type = "<<the_particle_label<<std::endl;
  //         other_events_flux<<"Event generation time"<<a_time<<std::endl;
  //       }
  //     }
  //     else {
  //       std::cout<< "ERREUR: Impossible d'ouvrir le fichier other_events.txt" <<std::endl;
  //     }
  //   }
  // }

  //! Reset the module
  virtual void reset();


protected:

  /// Give default values to specific class members
  void _set_defaults();

private:

  // The root output file:
  TFile * _sd_output_file_;

  // The root tree:
  TTree * _sd_tree_;
  double _time_;
  double _energy_;

  double _the_time_difference_ = 0;
  int _the_number_of_simulated_electrons_ = 0;
  double _number_of_internal_conversion_events_= 0;
  double _number_of_other_events_= 0;
  double _number_of_events_= 0;
  double _on_exponential_= 0;

  // Macro which automatically creates the interface needed
  // to enable the module to be loaded at runtime
  DPP_MODULE_REGISTRATION_INTERFACE(MySecondModule)
};
#endif // MYSECONDMODULE_HH
