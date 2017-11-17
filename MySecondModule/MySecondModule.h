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

  //functions
  double g_EnergyCut (double energy_sum, double energy_max, double energy_min);


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
  double _number_of_internal_conversion_events_=0;
  double _number_of_other_events_=0;
  double _number_of_events_=0;
  double _on_exponential_=0;

  // Macro which automatically creates the interface needed
  // to enable the module to be loaded at runtime
  DPP_MODULE_REGISTRATION_INTERFACE(MySecondModule)
};
#endif // MYSECONDMODULE_HH
