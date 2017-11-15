// tree_generator.cc

// Ourselves:
#include <snemo/analysis/tree_generator.h>

// Standard library:
#include <stdexcept>
#include <sstream>
#include <numeric>

// Third party:
// - Bayeux/datatools:
#include <datatools/clhep_units.h>
#include <datatools/service_manager.h>
// - Bayeux/mctools
#include <mctools/simulated_data.h>

// SuperNEMO event model
#include <snemo/datamodels/data_model.h>
#include <snemo/datamodels/event_header.h>
#include <snemo/datamodels/particle_track_data.h>
#include <falaise/snemo/datamodels/helix_trajectory_pattern.h>

namespace analysis {

  // Registration instantiation macro :
  DPP_MODULE_REGISTRATION_IMPLEMENT(tree_generator,
                                    "analysis::tree_generator");

  void tree_generator::_set_defaults()
  {
		_particle_track_tree_ = 0;
    return;
  }

  // Constructor :
  tree_generator::tree_generator(datatools::logger::priority logging_priority_)
    : dpp::base_module(logging_priority_)
  {
    _set_defaults();
    return;
  }

  // Destructor :
  tree_generator::~tree_generator()
  {
    if (is_initialized()) tree_generator::reset();
    return;
  }

  // Initialization :
  void tree_generator::initialize(const datatools::properties  & config_,
                                                  datatools::service_manager   & service_manager_,
                                                  dpp::module_handle_dict_type & module_dict_)
  {
    DT_THROW_IF(is_initialized(),
                std::logic_error,
                "Module '" << get_name() << "' is already initialized ! ");

    dpp::base_module::_common_initialize(config_);

		if (config_.has_key("Root_output_files")) {
        std::vector<std::string> output_files;
        config_.fetch("Root_output_files", output_files);

				DT_THROW_IF(output_files.size() != 1,
										std::logic_error,
										"Module '" << get_name() << "' can handle only 1 root file for now ! ");

				_root_output_file_ = new TFile (output_files[0].c_str(), "recreate");
		}

		// // Root tree
		_root_output_file_->cd();
		_particle_track_tree_ = new TTree("particle track tree", "particle track tree");
		_particle_track_tree_->Branch("run", &_run_,"run/I");
		_particle_track_tree_->Branch("event", &_event_,"event/I");
		_particle_track_tree_->Branch("ndf", &_ndf_,"ndf/I");
		_particle_track_tree_->Branch("chi2", &_chi2_,"chi2/D");
		_particle_track_tree_->Branch("radius", &_radius_,"radius/D");
		_particle_track_tree_->Branch("length", &_length_,"length/D");
		// _particle_track_tree_->Branch("energy", &_energy_,"energy/D");
		_particle_track_tree_->Branch("xve", &_xve_,"xve/D");
		_particle_track_tree_->Branch("yve", &_yve_,"yve/D");
		_particle_track_tree_->Branch("zve", &_zve_,"zve/D");
		_particle_track_tree_->Branch("xhe", &_xhe_,"xhe/D");
		_particle_track_tree_->Branch("yhe", &_yhe_,"yhe/D");
		_particle_track_tree_->Branch("zhe", &_zhe_,"zhe/D");
		_particle_track_tree_->Branch("xcal", &_xcal_,"xcal/D");
		_particle_track_tree_->Branch("ycal", &_ycal_,"ycal/D");
		_particle_track_tree_->Branch("zcal", &_zcal_,"zcal/D");

    // Tag the module as initialized :
    _set_initialized (true);
    return;
  }

  // Reset :
  void tree_generator::reset()
  {
    DT_THROW_IF(! is_initialized(),
                std::logic_error,
                "Module '" << get_name() << "' is not initialized !");

    // Dump result
    if (get_logging_priority() >= datatools::logger::PRIO_DEBUG) {
      DT_LOG_NOTICE(get_logging_priority (), "Magnetic field module dump: ");
      dump_result();
    }

		// Root tree
		_root_output_file_->cd();
		_particle_track_tree_->Write();
		_root_output_file_->Close();

    _set_defaults();

    // Tag the module as un-initialized :
    _set_initialized (false);
    return;
  }

  // Processing :
  dpp::base_module::process_status tree_generator::process(datatools::things & data_record_)
  {
    DT_LOG_TRACE(get_logging_priority(), "Entering...");
    DT_THROW_IF(! is_initialized(), std::logic_error,
                "Module '" << get_name() << "' is not initialized !");

    // Check if the 'event header' record bank is available :
    const std::string eh_label = snemo::datamodel::data_info::default_event_header_label();
    if (! data_record_.has(eh_label)) {
      DT_LOG_ERROR(get_logging_priority (), "Could not find any bank with label '"
                   << eh_label << "' !");
      return dpp::base_module::PROCESS_STOP;
    }
    const snemo::datamodel::event_header & eh
      = data_record_.get<snemo::datamodel::event_header>(eh_label);

    // Check if the 'simulated data' record bank is available :
    const std::string sd_label = snemo::datamodel::data_info::default_simulated_data_label();
    if (! data_record_.has(sd_label)) {
      DT_LOG_ERROR(get_logging_priority(), "Could not find any bank with label '"
                   << sd_label << "' !");
      return dpp::base_module::PROCESS_STOP;
    }
    const mctools::simulated_data & sd
      = data_record_.get<mctools::simulated_data>(sd_label);

    // Check if the 'particle track data' record bank is available :
    const std::string ptd_label = snemo::datamodel::data_info::default_particle_track_data_label();
    if (! data_record_.has(ptd_label)) {
      DT_LOG_ERROR(get_logging_priority (), "Could not find any bank with label '"
                   << ptd_label << "' !");
      return dpp::base_module::PROCESS_STOP;
    }
    const snemo::datamodel::particle_track_data & ptd
      = data_record_.get<snemo::datamodel::particle_track_data>(ptd_label);

    if (get_logging_priority() >= datatools::logger::PRIO_DEBUG) {
      DT_LOG_DEBUG(get_logging_priority(), "Event header : ");
      eh.tree_dump();
      DT_LOG_DEBUG(get_logging_priority(), "Simulated data : ");
      sd.tree_dump();
      DT_LOG_DEBUG(get_logging_priority(), "Particle track data : ");
      ptd.tree_dump();
    }

    // Loop over all saved particles
    const snemo::datamodel::particle_track_data::particle_collection_type & the_particles
      = ptd.get_particles();

    for (snemo::datamodel::particle_track_data::particle_collection_type::const_iterator
           iparticle = the_particles.begin();
         iparticle != the_particles.end(); ++iparticle) {
      const snemo::datamodel::particle_track & a_particle = iparticle->get();
      const snemo::datamodel::particle_track::vertex_collection_type & the_vertices
        = a_particle.get_vertices();

			/*
      geomtools::vector_3d delta;
      geomtools::invalidate(delta);
      for (snemo::datamodel::particle_track::vertex_collection_type::const_iterator
             ivertex = the_vertices.begin();
           ivertex != the_vertices.end(); ++ivertex) {
        const geomtools::blur_spot & a_vertex = ivertex->get();
        const datatools::properties & aux = a_vertex.get_auxiliaries();

        if (!aux.has_key(snemo::datamodel::particle_track::vertex_type_key())) {
          DT_LOG_WARNING(get_logging_priority(), "Current vertex has no vertex type !");
          continue;
        }
        std::string vname = aux.fetch_string(snemo::datamodel::particle_track::vertex_type_key());

        // Calculate delta vertex
        if (snemo::datamodel::particle_track::vertex_is_on_source_foil(a_vertex)) {
          delta = a_vertex.get_position() - sd.get_vertex();
        } else {
          // Getting the simulated hit inside the calorimeter
          if (! sd.has_step_hits(vname)) {
            DT_LOG_WARNING(get_logging_priority(), "Simulated data has not step hit associated to '" << vname << "' category ! Skip !");
            continue;
          }
          const mctools::simulated_data::hit_handle_collection_type & hits = sd.get_step_hits(vname);
          if (hits.size() != 1) {
            DT_LOG_WARNING(get_logging_priority(), "More than one energy deposit ! Skip !");
            continue;
          }
          // Get the first calorimeter simulated step hit i.e. the one at
          // the entrance of the calorimeter block
          const mctools::base_step_hit & the_step_hit = hits.front().get();
          const geomtools::geom_id & the_step_hit_gid = the_step_hit.get_geom_id();

          // Get the associated calorimeter list
          const snemo::datamodel::calibrated_calorimeter_hit::collection_type calos
            = a_particle.get_associated_calorimeter_hits();
          if (calos.size() != 1) {
            DT_LOG_WARNING(get_logging_priority(), "More than one calorimeter associated to the particle track ! Skip !");
            continue;
          }
          const geomtools::geom_id & the_calo_gid = calos.front().get().get_geom_id();
          if (! geomtools::geom_id::match (the_calo_gid, the_step_hit_gid)) {
            DT_LOG_WARNING(get_logging_priority(),
                           "Simulated calorimeter does not match associated calorimeter ! Skip !");
            continue;
          }

          // delta = a_vertex.get_position() - the_step_hit.get_position_start();
          // TODO: Use mean position value of the calo box deposit
          delta = a_vertex.get_position() - 0.5*(the_step_hit.get_position_start() +
                                                 the_step_hit.get_position_stop());
        }
        DT_LOG_DEBUG(get_logging_priority(), "Delta value = " << delta/CLHEP::mm << " mm");

        DT_THROW_IF(!geomtools::is_valid(delta), std::logic_error,
                    "Something gets wrong when vertex difference has been calculated");

        // Getting histogram pool
        mygsl::histogram_pool & a_pool = grab_histogram_pool();
        const std::string label[3] = { "x", "y", "z"};
        for (size_t i = 0; i < 3; i++) {
          std::ostringstream key;
          key << std::setw(4) << sd.get_primary_event().get_total_kinetic_energy()/CLHEP::keV << "keV_";
          std::ostringstream group;
          group << vname << "_" << label[i] << "_position";
          key << group.str();
          if (! a_pool.has(key.str())) {
            mygsl::histogram_1d & h = a_pool.add_1d(key.str(), "", group.str());
            datatools::properties hconfig;
            hconfig.store_string("mode", "mimic");
            hconfig.store_string("mimic.histogram_1d", "delta_" + label[i]);
            mygsl::histogram_pool::init_histo_1d(h, hconfig, &a_pool);
          }
          // Getting the current histogram
          mygsl::histogram_1d & a_histo = a_pool.grab_1d(key.str ());
          if (label[i] == "x") a_histo.fill(delta.x());
          if (label[i] == "y") a_histo.fill(delta.y());
          if (label[i] == "z") a_histo.fill(delta.z());
        }
      }// end of vertex list
			*/

			const geomtools::i_shape_1d & a_track_shape = a_particle.get_trajectory ().get_pattern ().get_shape ();
			//a_track_shape.tree_dump ();

			const snemo::datamodel::base_trajectory_pattern & a_track_pattern = a_particle.get_trajectory().get_pattern();

			// Retrieve helix trajectory
			const snemo::datamodel::helix_trajectory_pattern * ptr_helix = 0;
			if (a_track_pattern.get_pattern_id() == snemo::datamodel::helix_trajectory_pattern::pattern_id()) {
				ptr_helix = dynamic_cast<const snemo::datamodel::helix_trajectory_pattern *>(&a_track_pattern);
			}
			if (!ptr_helix) {
				DT_LOG_ERROR(get_logging_priority(), "Tracker trajectory is not an 'helix' !");
				return dpp::base_module::PROCESS_STOP;
			}

			// Get the associated calorimeter list
			const snemo::datamodel::calibrated_calorimeter_hit::collection_type calo_hits
				= a_particle.get_associated_calorimeter_hits();
			if (calo_hits.size() != 1) {
				DT_LOG_WARNING(get_logging_priority(), "More than one calorimeter associated to the particle track ! Skip !");
				continue;
			}

			// Output root tree
			_root_output_file_->cd();
			_run_ = eh.get_id ().get_run_number ();
			_event_ = eh.get_id ().get_event_number ();

			_chi2_ = a_particle.get_trajectory ().get_auxiliaries().fetch_real("chi2");
			_ndf_ = a_particle.get_trajectory ().get_auxiliaries().fetch_integer("ndof");

			_radius_ = ptr_helix->get_helix().get_radius();
			_length_ = ptr_helix->get_helix().get_length();

			// _energy_ = calo_hits.front().get().get_energy();

			_xve_ = sd.get_vertex().x();
			_yve_ = sd.get_vertex().y();
			_zve_ = sd.get_vertex().z();

			_xhe_ = ptr_helix->get_helix().get_center().x();
			_yhe_ = ptr_helix->get_helix().get_center().y();
			_zhe_ = ptr_helix->get_helix().get_center().z();

			_xcal_ = 0.;
			_ycal_ = 0.;
			_zcal_ = 0.;

			_particle_track_tree_->Fill();

    }// end of particle list

    DT_LOG_TRACE(get_logging_priority(), "Exiting.");
    return dpp::base_module::PROCESS_SUCCESS;
  }

  void tree_generator::dump_result(std::ostream      & out_,
																					 const std::string & title_,
																					 const std::string & indent_,
																					 bool inherit_) const
  {
    std::string indent;
    if (! indent_.empty ()) {
      indent = indent_;
    }
    if (!title_.empty ()) {
      out_ << indent << title_ << std::endl;
    }

    return;
  }

} // namespace analysis

// end of tree_generator.cc
/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
