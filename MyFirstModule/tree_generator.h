/* tree_generator.h
 * Author(s)     : Steven Calvez <calvez@lal.in2p3.fr>
 * Creation date : 2016-01-22
 * Last modified : 2016-01-22
 *
 * Copyright (C) 2015-2016 Steven Calvez <calvez@lal.in2p3.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *
 * Description:
 *
 * A dedicated module to study the SuperNEMO magnetic field.
 *
 * History:
 *
 */

#ifndef ANALYSIS_TREE_GENERATOR_H
#define ANALYSIS_TREE_GENERATOR_H 1

// Data processing module abstract base class
#include <dpp/base_module.h>

// Root objects
#include <TFile.h>
#include <TTree.h>

namespace analysis {

  class tree_generator : public dpp::base_module
  {
  public:

    /// Constructor
    tree_generator(datatools::logger::priority = datatools::logger::PRIO_FATAL);

    /// Destructor
    virtual ~tree_generator();

    /// Initialization
    virtual void initialize(const datatools::properties  & config_,
                            datatools::service_manager   & service_manager_,
                            dpp::module_handle_dict_type & module_dict_);

    /// Reset
    virtual void reset();

    /// Data record processing
    virtual process_status process(datatools::things & data_);

    /// Dump
    void dump_result (std::ostream      & out_    = std::clog,
                      const std::string & title_  = "",
                      const std::string & indent_ = "",
                      bool inherit_               = false) const;

  protected:

    /// Give default values to specific class members.
    void _set_defaults();

  private:

		// The root output file:
		TFile * _root_output_file_;

    // The root tree:
		TTree * _particle_track_tree_;
		int _event_;
		int _ndf_;
		double _chi2_;
		double _radius_;
		double _length_;
		double _energy_;
		double _xve_, _yve_, _zve_;
		double _xhe_, _yhe_, _zhe_;
		double _xcal_, _ycal_, _zcal_;

    // Macro to automate the registration of the module :
    DPP_MODULE_REGISTRATION_INTERFACE(tree_generator);
  };

} // namespace analysis

#endif // ANALYSIS_TREE_GENERATOR_H

// end of tree_generator.h
/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
