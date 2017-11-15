//! \file    MyFirstModule.h
//! \brief   Example processing module for flreconstruct
//! \details Process a things object
#ifndef MYFIRSTMODULE_HH
#define MYFIRSTMODULE_HH
// Standard Library
// Third Party
// - Bayeux
#include <bayeux/dpp/base_module.h>

// Root objects
#include <TFile.h>
#include <TTree.h>

#include <falaise/snemo/datamodels/base_topology_pattern.h>


class MyFirstModule : public dpp::base_module
{
public:

  //! Construct module
  MyFirstModule();

  //! Destructor
  virtual ~MyFirstModule();

  //! Configure the module
  virtual void initialize(const datatools::properties& myConfig,
                          datatools::service_manager& flServices,
                          dpp::module_handle_dict_type& moduleDict);

  // Return electron vertex location
  std::string get_electron_vertex_location() const;

  // Check electron vertex location validity
  bool has_electron_vertex_location() const;

  //! Process supplied data record
  virtual dpp::base_module::process_status process(datatools::things& workItem);

  //! Reset the module
  virtual void reset();


protected:

  /// Give default values to specific class members
  void _set_defaults();

private:

  // std::string good_internal_probability;
  // std::string tof_e1_e2;
  // std::string bad_external_probability;
  // std::string tof_e1_e2;
  // std::string good_vertices_probability;
  // std::string vertex_e1_e2;
  // std::string good_vertices_distance;
  // std::string vertex_e1_e2;
  // std::string vertex_on_foil;
  // std::string vertex_e1_e2;

  double _dummy_;
  std::vector<double> _calo_times_;
  std::vector<double> _calo_energies_;

  // The root output file:
  TFile * _root_output_file_;

  // The root tree:
  TTree * _calorimeter_hits_tree_;
  double _energy_sum_;
  double _energy_maximal_;
  double _energy_minimal_;
  double _internal_probability_;
  double _energy_;
  double _time_;

  // Macro which automatically creates the interface needed
  // to enable the module to be loaded at runtime
  DPP_MODULE_REGISTRATION_INTERFACE(MyFirstModule)
};
#endif // MYFIRSTMODULE_HH
