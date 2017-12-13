/*
Copyright (c) 2015, Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "boost/program_options.hpp"
#include <iostream>
#include <string>
#include <cstring>

#include "faultsim.hh"
#include "ConfigParser.hh"
#include "GroupDomain.hh"
#include "GroupDomain_dimm.hh"
#include "GroupDomain_cube.hh"
#include "DRAMDomain.hh"
#include "ChipKillRepair.hh"
#include "ChipKillRepair_cube.hh"
#include "BCHRepair_cube.hh"
#include "CubeRAIDRepair.hh"
#include "BCHRepair.hh"
#include "Simulation.hh"
#include "EventSimulation.hh"
#include "Settings.hh"

void printBanner( void );
GroupDomain* genModuleDIMM(uint num);
GroupDomain* genModule3D( void );

namespace {
const size_t ERROR_IN_COMMAND_LINE = 1;
const size_t SUCCESS = 0;
const size_t ERROR_UNHANDLED_EXCEPTION = 2;

} // namespace

void printBanner( void )
{
	cout << "# --------------------------------------------------------------------------------\n";
	cout << "# FAULTSIM (v0.1 alpha) - A Fast, Configurable Memory Resilience Simulator\n";
	cout << "# (c) 2013-2015 Advanced Micro Devices Inc.\n";
	cout << "# --------------------------------------------------------------------------------\n\n";
}

struct Settings settings;

int main(int argc, char** argv) {

    std::string chain="NULL";
    printBanner();

	try {
		/** Define and parse the program options
		 */
		namespace po = boost::program_options;
		po::options_description desc("Options");

		/** Prashant Adding Options for higher end BCH repair codes in the "mode" field and a test field to do primitive testing of cases */

		desc.add_options()("help", "Print help messages")
										  ("outfile", po::value<std::string>(&settings.output_file)->required(), "Output file name")
                                          ("configfile",po::value<std::string>(&chain),"Indicate .ini configuration file to use");

		po::variables_map vm;
		try {
			po::store(po::parse_command_line(argc, argv, desc), vm); // can throw

			/** --help option
			 */
			if ( (argc == 1) || vm.count("help")) {
				std::cout << "FaultSim" << std::endl << desc << std::endl;
				return SUCCESS;
			}

			po::notify(vm); // throws on error, so do after help in case
			// there are any problems
		} catch (po::error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return ERROR_IN_COMMAND_LINE;
		}

		// application code here //

	} catch (std::exception& e) {
		std::cerr << "Unhandled Exception reached the top of main: " << e.what()
								<< ", application will now exit" << std::endl;
		return ERROR_UNHANDLED_EXCEPTION;

	}
    cout<<"The selected config file is: "<<chain<<endl;
    char * config_opt = new char [chain.size()+1];
    strcpy (config_opt,chain.c_str());


	parser(config_opt);
    delete [] config_opt;

    // Build the physical memory organization and attach ECC scheme /////
    GroupDomain **module = (GroupDomain **)calloc(settings.module,sizeof(GroupDomain *));
    //module=(module **)calloc(settings.module,sizeof(module *));
    //GroupDomain *module1 = NULL;
    //GroupDomain *module2 = NULL;

    if( settings.organization == MO_DIMM ) {
      for (int i=0;i<settings.module;i++)
    	module[i] = genModuleDIMM(i);
      //module2 = genModuleDIMM();
    } else if( settings.organization == MO_3D ) {
      for (int i=0;i<settings.module;i++)
    	module[i] = genModule3D();
    }

    // Configure simulator ///////////////////////////////////////////////
    Simulation *sim_temp;

    // Simulator settings are as follows: 
    // a. The setting.interval_s (in seconds) indicates the granularity of inserting faults 
    // (not used in Event Based Simulator). 
    // b. The setting.scrub_s (in seconds) indicates the granularity of scrubbing transient faults.
    // c. The setting.fit_factor indicates the multiplicative factor for fit_rates. 
    // d. The setting.debug will enable debug messages
    // e. The setting.continue_running will enable uses to continue running even if an uncorrectable error occurs 
    // (until an undetectable error occurs.
    // f. The settings.output_bucket_s wil bucket system failure times
    // NOTE: The test_mode setting allows the user to inject specific faults at very FIT rates. This enables the user to test their
    // ECC technique and also stress corner cases for fault specific ECC.
    // NOTE: The test_mode setting is currently not implemented in the Event Based Simulator

    if( settings.sim_mode == 1 ) {
    	sim_temp = (new Simulation( settings.interval_s, settings.scrub_s, settings.fit_factor, settings.test_mode,
                                  settings.debug,settings.continue_running, settings.output_bucket_s,settings.turning_point )); //xiao:add turning_point
    } else if( settings.sim_mode == 2 ) {
    	sim_temp = (new EventSimulation( settings.interval_s, settings.scrub_s, settings.fit_factor, settings.test_mode,
                                       settings.debug,settings.continue_running, settings.output_bucket_s, settings.turning_point));
    } else {
    	cout << "ERROR: Invalid sim_mode option (must be 1 (interval-based) or 2 (event-driven))\n";
    	exit(0);
    }

    Simulation &sim = *sim_temp;

    // Run simulator //////////////////////////////////////////////////
    for (int i=0;i<settings.module;i++)
    sim.addDomain( module[i] );    // register the top-level memory object with the simulation engine
    //sim.addDomain( module2 );
    sim.init( settings.max_s );	// one-time set-up that does FIT rate scaling based on interval
    sim.simulate( settings.max_s, settings.n_sims, settings.verbose, settings.output_file);
    sim.printStats();

	return SUCCESS;

}
/*
 * Simulate a DIMM module
 */

// char *int2char(int x)
// {
//     int i;
//     char a[]=NULL;
//     while (x)
//     {
//         i=x%10;
//         x/=10;
//         strcat(x+"0",a);
//     }
//     return a;
// }
GroupDomain* genModuleDIMM(uint num)
{
	GroupDomain *dimm0;

	// Create a DIMM or a CUBE
	// settings.data_block_bits is the number of bits per transaction when you create a DIMM


  //string
  char module_name[]="MODULE0";
  //char num1[]="0";
  module_name[strlen(module_name)-1]+=num;
  //int2char(num);
  //strcat(num1,module_name);
	dimm0 = new GroupDomain_dimm( module_name, settings.chips_per_rank, settings.banks, settings.data_block_bits );

	for( uint32_t i = 0; i < settings.chips_per_rank; i++ ) {
		char buf[20];
		sprintf( buf, "MODULE%d.DRAM%d",num, i );
		DRAMDomain *dram0 = new DRAMDomain( buf, settings.chip_bus_bits, settings.ranks, settings.banks, settings.rows, settings.cols );

    if (settings.type==TY_DRAM)
    {
        if (settings.faultmode == FM_UNIFORM_BIT )
        {
            if( settings.enable_transient ) dram0->setFIT( DRAM_1BIT, 1, 33.05 );
            if( settings.enable_permanent ) dram0->setFIT( DRAM_1BIT, 0, 33.05 );
        }
        else if( settings.faultmode == FM_JAGUAR )
        {
            if( settings.enable_transient )
            {
                dram0->setFIT( DRAM_1BIT, 1, 14.2 );
                dram0->setFIT( DRAM_1WORD, 1, 1.4 );
                dram0->setFIT( DRAM_1COL, 1, 1.4 );
                dram0->setFIT( DRAM_1ROW, 1, 0.2 );
                dram0->setFIT( DRAM_1BANK, 1, 0.8 );
                dram0->setFIT( DRAM_NBANK, 1, 0.3 );
                dram0->setFIT( DRAM_NRANK, 1, 0.9 );
            }

            if( settings.enable_permanent )
            {
                dram0->setFIT( DRAM_1BIT, 0, 18.6 );
                dram0->setFIT( DRAM_1WORD, 0, 0.3 );
                dram0->setFIT( DRAM_1COL, 0, 5.6 );
                dram0->setFIT( DRAM_1ROW, 0, 8.2 );
                dram0->setFIT( DRAM_1BANK, 0, 10.0 );
                dram0->setFIT( DRAM_NBANK, 0, 1.4 );
                dram0->setFIT( DRAM_NRANK, 0, 2.8 );
            }
        }
        else
        {
            assert(0);
        }
    }
    else if (settings.type==TY_PCM)
    {
        if (settings.faultmode == FM_UNIFORM_BIT )
        {
            if( settings.enable_transient ) dram0->setFIT( DRAM_1BIT, 1, double(SLC_PCM_FIT/2) );
            if( settings.enable_permanent ) dram0->setFIT( DRAM_1BIT, 0, double(SLC_PCM_FIT/2) );
        }
        else if( settings.faultmode == FM_JAGUAR )
        {
            if( settings.enable_transient )
            {
                dram0->setFIT( DRAM_1BIT, 1, 14.2 );
                dram0->setFIT( DRAM_1WORD, 1, 1.4 );
                dram0->setFIT( DRAM_1COL, 1, 1.4 );
                dram0->setFIT( DRAM_1ROW, 1, 0.2 );
                dram0->setFIT( DRAM_1BANK, 1, 0.8 );
                dram0->setFIT( DRAM_NBANK, 1, 0.3 );
                dram0->setFIT( DRAM_NRANK, 1, 0.9 );
            }

            if( settings.enable_permanent )
            {
                dram0->setFIT( DRAM_1BIT, 0, 18.6 );
                dram0->setFIT( DRAM_1WORD, 0, 0.3 );
                dram0->setFIT( DRAM_1COL, 0, 5.6 );
                dram0->setFIT( DRAM_1ROW, 0, 8.2 );
                dram0->setFIT( DRAM_1BANK, 0, 10.0 );
                dram0->setFIT( DRAM_NBANK, 0, 1.4 );
                dram0->setFIT( DRAM_NRANK, 0, 2.8 );
            }
        }
        else
        {
            assert(0);
        }
    }
    else assert(0);

		dimm0->addDomain( dram0, i );
	}

	//Add the 2D Repair Schemes
	if( settings.repairmode == 0 ) {
		// do nothing (no ECC)
	} else if( settings.repairmode == 1 ) {
		ChipKillRepair *ck0 = new ChipKillRepair( string("CK1"), 1, 2 );
		dimm0->addRepair( ck0 );
	} else if( settings.repairmode == 2 ) {
		ChipKillRepair *ck0 = new ChipKillRepair( string("CK2"), 2, 4 );
		dimm0->addRepair( ck0 );
	} else if( settings.repairmode == 3 ) {
		BCHRepair *bch0 = new BCHRepair( string("SECDED"), 1, 2, 4 );
		dimm0->addRepair( bch0 );
	} else if( settings.repairmode == 4 ) {
		BCHRepair *bch1 = new BCHRepair( string("3EC4ED"), 3, 4, 4 );
		dimm0->addRepair( bch1 ); //Repair from Fault Domain
	} else if( settings.repairmode == 5 ) {
		BCHRepair *bch2 = new BCHRepair( string("6EC7ED"), 6, 7, 4 );
		dimm0->addRepair( bch2 );
	} else {
		assert(0);
	}

	return dimm0;
}

GroupDomain *genModule3D( void )
{
	GroupDomain *stack0;

	// Create a stack or a CUBE
	// settings.data_block_bits is the number of bits per transaction when you create a Cube
	         
	stack0 = new GroupDomain_cube( "MODULE0",1,settings.chips_per_rank,settings.banks,settings.data_block_bits,settings.cube_addr_dec_depth, settings.cube_ecc_tsv, settings.cube_redun_tsv, settings.enable_tsv);

	//Set FIT rates for TSVs, these are set at the GroupDomain level as these are common to the entire cube
	stack0->setFIT_TSV( 1, settings.tsv_fit );
	stack0->setFIT_TSV( 0, settings.tsv_fit );

	// Set FIT rates for different granularity for all devices in the module and add devices into the module
	double DRAM_nrank_fit_trans = 0;
	double DRAM_nrank_fit_perm = 0;

	// Rank FIT rates cannot be directly translated to 3D stack
	DRAM_nrank_fit_trans = 0.0;
	DRAM_nrank_fit_perm = 0.0;

	for( uint32_t i = 0; i < settings.chips_per_rank; i++ ) {
		char buf[20];
		sprintf( buf, "MODULE0.DRAM%d", i );
		DRAMDomain *dram0 = new DRAMDomain( buf, settings.chip_bus_bits, settings.ranks, settings.banks, settings.rows, settings.cols );

		if( settings.faultmode == FM_UNIFORM_BIT ) {
			// use a default FIT rate equal to probability of any Jaguar fault
			if( settings.enable_transient ) dram0->setFIT( DRAM_1BIT, 1, 33.05 );
			if( settings.enable_permanent ) dram0->setFIT( DRAM_1BIT, 0, 33.05 );
		} else if( settings.faultmode == FM_JAGUAR ) {
			if( settings.enable_transient ) {
				dram0->setFIT( DRAM_1BIT, 1, 14.2 );
				dram0->setFIT( DRAM_1WORD, 1, 1.4 );
				dram0->setFIT( DRAM_1COL, 1, 1.4 );
				dram0->setFIT( DRAM_1ROW, 1, 0.2 );
				dram0->setFIT( DRAM_1BANK, 1, 0.8 );
				dram0->setFIT( DRAM_NBANK, 1, 0.3 );
				dram0->setFIT( DRAM_NRANK, 1, DRAM_nrank_fit_trans );
			}

			if( settings.enable_permanent ) {
				dram0->setFIT( DRAM_1BIT, 0, 18.6 );
				dram0->setFIT( DRAM_1WORD, 0, 0.3 );
				dram0->setFIT( DRAM_1COL, 0, 5.6 );
				dram0->setFIT( DRAM_1ROW, 0, 8.2 );
				dram0->setFIT( DRAM_1BANK, 0, 10.0 );
				dram0->setFIT( DRAM_NBANK, 0, 1.4 );
				dram0->setFIT( DRAM_NRANK, 0, DRAM_nrank_fit_perm );
			}
		} else {
			assert(0);
		}

		stack0->addDomain( dram0, i );
	}

	if( settings.repairmode == 1 ) {
		ChipKillRepair_cube *ck0 = new ChipKillRepair_cube( string("CK1"), 1, 2, stack0);
		stack0->addRepair( ck0 );
	} else if( settings.repairmode == 2 ) {
		CubeRAIDRepair *ck1 = new CubeRAIDRepair( string("RAID"), 1, 2, settings.data_block_bits );
		stack0->addRepair( ck1 ); //settings.data_block_bits used as RAID is computed over 512 bits (in our design)
	} else if( settings.repairmode == 3 ) {
		BCHRepair_cube *bch0 = new BCHRepair_cube( string("SECDED"), 1, 2, settings.data_block_bits );
		stack0->addRepair( bch0 ); //settings.data_block_bits used as SECDED/3EC4ED/6EC7ED is computed over 512 bits (in our design)
	} else if( settings.repairmode == 4 ) {
		BCHRepair_cube *bch1 = new BCHRepair_cube( string("3EC4ED"), 3, 4, settings.data_block_bits );
		stack0->addRepair( bch1 ); //Repair from Fault Domain
	} else if( settings.repairmode == 5 ) {
		BCHRepair_cube *bch2 = new BCHRepair_cube( string("6EC7ED"), 6, 7, settings.data_block_bits );
		stack0->addRepair( bch2 );
	}
	else if( settings.repairmode == 6 ) {
		assert(0);
	}

	return stack0;
}
