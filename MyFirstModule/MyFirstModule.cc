// - Implementation of MyFirstModule
// Ourselves
#include "MyFirstModule.h"
// Standard Library
// Third Party
// - A
// This Project
#include <falaise/snemo/datamodels/pid_utils.h>
#include <snemo/datamodels/particle_track_data.h>
#include <snemo/datamodels/calibrated_data.h>
#include <snemo/datamodels/calibrated_calorimeter_hit.h>
#include <snemo/datamodels/topology_data.h>

#include <snemo/datamodels/topology_exporter.h>
#include <snemo/datamodels/export_topology.h>
#include <snemo/datamodels/base_data_bank_exporter.h>

#include <snemo/datamodels/topology_2e_pattern.h>
#include <snemo/datamodels/topology_1e_pattern.h>

#include <snemo/datamodels/particle_track.h>

//#include <snemo/datamodels/vertex_measurement.h>

//#include <datatools/things_macros.h>
// #include <snemo/cuts/topology_data_cut.h>


// Macro which automatically implements the interface needed
// to enable the module to be loaded at runtime
// The first argument is the typename
// The second is the string key used to access the module in pipeline
// scripts. This must be globally unique.
DPP_MODULE_REGISTRATION_IMPLEMENT(MyFirstModule,"MyFirstModule")

// Construct
MyFirstModule::MyFirstModule() : dpp::base_module()
{
  this->_set_defaults();
}

// Destruct
MyFirstModule::~MyFirstModule() {
  // MUST reset module at destruction
  this->reset();
}

// Initialize
void MyFirstModule::initialize(const datatools::properties& setup_,
                               datatools::service_manager& /*flServices*/,
                               dpp::module_handle_dict_type& /*moduleDict*/) {

  DT_THROW_IF(is_initialized(), std::logic_error,
              "Module '" << get_name () << "' is not initialized !");

  dpp::base_module::_common_initialize(setup_);

  // DT_LOG_TRACE(get_logging_priority(), "Entering...");
  // DT_LOG_DEBUG(get_logging_priority(), "dummy = " << _dummy_);

  //setup _dummy_ with configuration file
  // if (setup_.has_key("dummy")) {
  //   _dummy_ = setup_.fetch_real("dummy");
  // }

  // if (setup_.has_key("Root_output_files")) {
  //   std::vector<std::string> output_files;
  //   setup_.fetch("Root_output_files", output_files);

  //   DT_THROW_IF(output_files.size() != 1,
  //               std::logic_error,
  //               "Module '" << get_name() << "' can handle only 1 root file for now ! ");
  // }

  _root_output_file_ = new TFile ("td_tree.root", "recreate", "Output file of Simulation data");

  //Root tree
  _root_output_file_->cd();
  _calorimeter_hits_tree_ = new TTree("calorimeter hit tree", "calorimeter hit tree");
  _calorimeter_hits_tree_->Branch("energy", &_energy_,"energy/D");
  _calorimeter_hits_tree_->Branch("time", &_time_,"time/D");
  _calorimeter_hits_tree_->Branch("energy_minimal", &_energy_minimal_,"energy_minimal/D");
  _calorimeter_hits_tree_->Branch("energy_maximal", &_energy_maximal_,"energy_maximal/D");
  _calorimeter_hits_tree_->Branch("energy_sum", &_energy_sum_,"energy_sum/D");

  // DT_LOG_DEBUG(get_logging_priority(), "dummy = " << _dummy_);
  // DT_LOG_TRACE(get_logging_priority(), "Exiting.");
  this->_set_initialized(true);
}

// Process
dpp::base_module::process_status
MyFirstModule::process(datatools::things& data_record_) {
  DT_THROW_IF(! is_initialized(), std::logic_error,
              "Module '" << get_name () << "' is not initialized !");

  // data_record_.tree_dump();

  // if (data_record_.has("PTD")) {
  // const snemo::datamodel::particle_track_data & a_ptd
  //   = data_record_.get<snemo::datamodel::particle_track_data>("PTD");
  // a_ptd.tree_dump();
  // }

  //topology data (storing the calibrated datas)
  if (data_record_.has("TD")) {
    const snemo::datamodel::topology_data & a_td
      =  data_record_.get<snemo::datamodel::topology_data>("TD");

    //checking the topology pattern
    if (! a_td.has_pattern()) {
      DT_LOG_DEBUG(get_logging_priority(), "No topology pattern stored!");
      return PROCESS_CONTINUE;
    }
    ////////////////////////////////////////////
    // //1 electron pattern
    if (! a_td.has_pattern_as<snemo::datamodel::topology_1e_pattern>()) {
      DT_LOG_DEBUG(get_logging_priority(), "No '1e' topology pattern stored!");
      return PROCESS_CONTINUE;
    }

    const snemo::datamodel::topology_1e_pattern & a_1e_topology
      =a_td.get_pattern_as<snemo::datamodel::topology_1e_pattern>();

    if (a_1e_topology.has_electron_vertex_location()) {
      const std::string a_vertex_location
        =a_1e_topology.get_electron_vertex_location();
      DT_LOG_DEBUG(get_logging_priority(), "Electron vertex location : " << a_vertex_location);

      if (a_1e_topology.has_electron_track()
          && a_1e_topology.has_electron_energy()
          && a_vertex_location == "foil") {
        const double & a_electron_energy
          =a_1e_topology.get_electron_energy();
        DT_LOG_DEBUG(get_logging_priority(), "The TD has an electron track with an energy "<<a_electron_energy<<"MeV");

        //Output root tree
        _root_output_file_->cd();
        _energy_ = a_electron_energy;
        _calorimeter_hits_tree_->Fill();

        //calibrated data
        const snemo::datamodel::calibrated_data & a_cd
          = data_record_.get<snemo::datamodel::calibrated_data>("CD");
        if (a_cd.has_calibrated_calorimeter_hits()) {
          DT_LOG_DEBUG(get_logging_priority(), "Calibrated data has calorimeter hits!");

          const snemo::datamodel::calibrated_data::calorimeter_hit_collection_type & the_calo_hits
            = a_cd.calibrated_calorimeter_hits();

          //DT_LOG_DEBUG(get_logging_priority(), "Number of calo. hits = " << the_calo_hits.size());

          for (const auto & it : the_calo_hits) {
            // get the calorimeter hits
            const snemo::datamodel::calibrated_calorimeter_hit & a_calo = it.get();

            //get the calorimeter hit times
            const double a_time = a_calo.get_time();

            _root_output_file_->cd();
            _time_ = a_time;
            _calorimeter_hits_tree_->Fill();
          }
        }
      }
    }
    ////////////////////////////////////////////
    //2 electrons pattern
    // if (! a_td.has_pattern_as<snemo::datamodel::topology_2e_pattern>()) {
    //   DT_LOG_DEBUG(get_logging_priority(), "No '2e' topology pattern stored!");
    //   return PROCESS_CONTINUE;
    // }

    // const snemo::datamodel::topology_2e_pattern & a_2e_topology
    //   = a_td.get_pattern_as<snemo::datamodel::topology_2e_pattern>();

    // if (a_2e_topology.has_electrons_vertex_location()) {
    //   const std::string a_vertex_location_2e
    //     = a_2e_topology.get_electrons_vertex_location();
    //   DT_LOG_DEBUG(get_logging_priority(), "Electron vertex location : " << a_vertex_location_2e);
    //   if (a_2e_topology.has_electrons_energy()) {
    //     // if (a_2e_topology.has_electron_track()) {
    //     //   DT_LOG_DEBUG(get_logging_priority(), "The TD has a 2e pattern");
    //     if (a_2e_topology.has_electron_minimal_energy()
    //         &&a_2e_topology.has_electron_maximal_energy()
    //         &&a_2e_topology.has_electrons_internal_probability()) {
    //       const double & a_minimal_energy
    //         =a_2e_topology.get_electron_minimal_energy();
    //       std::cout<<"electron minimal energy"<<a_minimal_energy<<std::endl;

    //       const double & a_maximal_energy
    //         =a_2e_topology.get_electron_maximal_energy();
    //       std::cout<<"electron maximal energy"<<a_maximal_energy<<std::endl;

    //       // const double & a_internal_probability
    //       //   =a_2e_topology.get_electrons_internal_probability();
    //       // std::cout<<"electron internal probability"<<a_internal_probability<<std::endl;

    //       const double & a_energy_sum
    //         =a_2e_topology.get_electrons_energy_sum();
    //       std::cout<<"electron energy sum"<<a_energy_sum<<std::endl;

    //       _root_output_file_->cd();
    //       _energy_minimal_=a_minimal_energy;
    //       _energy_maximal_=a_maximal_energy;
    //       _energy_sum_=a_energy_sum;
    //       // _internal_probability_=a_internal_probability;
    //       _calorimeter_hits_tree_->Fill();
    //     }
    //     // if (a_2e_topology.has_electron_maximal_energy()) {
    //     //   const double & a_maximal_energy
    //     //     =a_2e_topology.get_electron_maximal_energy();
    //     //   std::cout<<"electron maximal energy"<<a_maximal_energy<<std::endl;
    //     // }
    //     // if (a_2e_topology.has_electrons_internal_probability()) {
    //     //   const double & a_internal_probability
    //     //     =a_2e_topology.get_electrons_internal_probability();
    //     //   std::cout<<"electron internal probability"<<a_internal_probability<<std::endl;
    //     // }


    //     //calibrated data
    //     if (data_record_.has("CD")) {
    //       const snemo::datamodel::calibrated_data & a_cd
    //         = data_record_.get<snemo::datamodel::calibrated_data>("CD");
    //       if (a_cd.has_calibrated_calorimeter_hits()) {
    //         DT_LOG_DEBUG(get_logging_priority(), "Calibrated data has calorimeter hits!");

    //         const snemo::datamodel::calibrated_data::calorimeter_hit_collection_type & the_calo_hits
    //           = a_cd.calibrated_calorimeter_hits();

    //         //DT_LOG_DEBUG(get_logging_priority(), "Number of calo. hits = " << the_calo_hits.size());

    //         for (const auto & it : the_calo_hits) {
    //           // get the calorimeter hits
    //           const snemo::datamodel::calibrated_calorimeter_hit & a_calo = it.get();

    //           //get the calorimeter hit times
    //           const double a_time = a_calo.get_time();

    //           _root_output_file_->cd();
    //           _time_ = a_time;
    //           _calorimeter_hits_tree_->Fill();
    //         }
    //       }
    //     }
    //   }
    // }



    // const std::string & a_track
    //   =a_1e_topology.get_pattern_id<snemo::datamodel::particle_track>();

    // if (a_td.has_track_id()) {
    //   std::cout<<"Track!***********************"<<std::endl;
    //   DT_LOG_DEBUG(get_logging_priority(),"No valid track id!")
    // }

    // const datatools::properties & td_aux = a_td.get_auxiliaries();
    // const std::string & a_classification = td_aux.fetch_string(snemo::datamodel::pid_utils::classification_label_key());
    // std::cout<<a_classification<<std::endl;

    // if (a_classification == snemo::datamodel::topology_1e_pattern::pattern_id()) {
    //   std::cout<<"*********entering in 1e loop***********"<<std::endl;

    //   std::cout<<"*********after a_vertex_location declaration***********"<<std::endl;

    //   DT_THROW_IF(! has_electron_vertex_location(), std::logic_error, "No electron vertex measurement stored !");

    //   a_vertex_location.tree_dump();

    //   //electrons_energy_sum = a_2e_pattern.get_electrons_energy_sum();

    //   // for (snemo::datamodel::calibrated_data::calorimeter_hit_collection_type::const_iterator it = the_calo_hits.begin();
    //   //      it != the_calo_hits.end(); ++it) {
    //   for (const auto & it : the_calo_hits) {
    //     // get the calorimeter hits
    //     const snemo::datamodel::calibrated_calorimeter_hit & a_calo = it.get();
    //     //a_calo.tree_dump();

    //     //get the calorimeter hit times
    //     const double a_time = a_calo.get_time();
    //     //std::cout<<"The calo. hit time is: "<<a_time<<" ns"<<std::endl;

    //     //get the calorimeter hit energies
    //     const double a_energy = a_calo.get_energy();
    //     //std::cout<<"The calo. hit energy is: "<<a_energy<<" MeV"<<std::endl;

    //     // Output root tree
    //     _root_output_file_->cd();

    //     _time_ = a_time;
    //     _energy_ = a_energy;

    //     _calorimeter_hits_tree_->Fill();
    //   }
    // }
  } //*********ends the track if loop


  //a_cd.tree_dump();

  // if (data_record_.has("TD"))
  //   {
  // std::cout<< "a_td" <<std::endl;
  // const snemo::datamodel::topology_data & a_td
  //       =  data_record_.get<snemo::datamodel::topology_data>("TD");
  // const datatools::properties & td_aux = a_td.get_auxiliaries();
  // const std::string & a_classification = td_aux.fetch_string(snemo::datamodel::pid_utils::classification_label_key());
  // std::cout<<a_classification<<std::endl;
  // }

  //  std::cout << "MyFirstModule::process called!" << std::endl;
  // MUST return a status, see ref dpp::base_module::process_status
  return PROCESS_OK;
}

// Reset
void MyFirstModule::reset()
{
  // Root tree
  _root_output_file_->cd();
  _calorimeter_hits_tree_->Write();
  _root_output_file_->Close();

  _set_defaults();

  // Tag MyFirstModule as un-initialized
  this->_set_initialized(false);
}

//Set defaults values of data members
void MyFirstModule::_set_defaults() {
  _dummy_ = 0.0;
}
