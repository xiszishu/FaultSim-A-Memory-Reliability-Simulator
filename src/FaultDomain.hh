/*
 * FAULTSIM: A Fast, Configurable Memory Resilience Simulator
 * (c) 2013-2015 Advanced Micro Devices, Inc.
 *
 */

#ifndef FAULTDOMAIN_HH_
#define FAULTDOMAIN_HH_

#include <list>
#include <vector>
#include <string>
#include "FaultRange.hh"
#include "dram_common.hh"
class RepairScheme;

using namespace std;

class FaultDomain
{
public:
	FaultDomain( const char *name );

	string getName( void );
	uint64_t getFaultCountTrans( void );
	uint64_t getFaultCountPerm( void );
	uint64_t getFaultCountUncorrected( void );
	uint64_t getFaultCountUndetected( void );
	uint64_t getFailedSimCount( void );

	virtual int update(uint test_mode_t);	// perform one iteration ; Prashant: Changed the update to return a non-void value
	virtual void repair( uint64_t &n_undetectable, uint64_t &n_uncorrectable );
	virtual uint64_t fill_repl(void);
	virtual void scrub( void );
	void addDomain( FaultDomain *domain, uint32_t domaincounter);
	void addRepair( RepairScheme *repair );
	// set up before first simulation run
	virtual void init( uint64_t interval, uint64_t sim_seconds, double m_fit_factor );
	// accrue simulation-level statistics at end of each sim run
	virtual void finalize( void );
	// reset after each sim run
	virtual void reset( void );
	virtual void dumpState( void );
	void setDebug( bool dbg );
	void setFIT_TSV(bool isTransient_TSV, double FIT_TSV );
	void update_cube();

	list<FaultDomain*> *getChildren( void );
	virtual void resetStats( void );
	virtual void printStats( void );	// output end-of-run stats

//private:
	string m_name;
//3D memory variables
	uint64_t cube_model_enable;
	uint64_t cube_addr_dec_depth;
	double tsv_transientFIT;
	double tsv_permanentFIT;
	uint64_t tsv_n_faults_transientFIT_class;
	uint64_t tsv_n_faults_permanentFIT_class;
	uint64_t chips;
	uint64_t banks;
	uint64_t burst_size;
	uint64_t cube_ecc_tsv;
	uint64_t cube_redun_tsv;
	uint64_t cube_data_tsv;
	bool *tsv_bitmap;
	uint64_t *tsv_info;
	uint64_t total_addr_tsv;
	uint64_t total_tsv;
	bool tsv_shared_accross_chips;
	uint64_t children_counter;
	uint64_t enable_tsv;
	//Static Arrays for ISCA2014
	uint64_t tsv_swapped_hc[9];
	uint64_t tsv_swapped_vc[8];
	uint64_t tsv_swapped_mc[9][8];
	// End 3D memory variable declaration

	bool visited;	// used during graph traversal algorithms

	bool debug;	// debug mode

	// per-simulation run statistics
	uint64_t n_faults_transient;
	uint64_t n_faults_permanent;
	uint64_t n_errors_uncorrected;
	uint64_t n_errors_undetected;

	uint64_t m_interval, m_sim_seconds, m_fit_factor;

	// cross-simulation overall program run statistics
	uint64_t stat_n_simulations, stat_n_failures, stat_n_failures_undetected, stat_n_failures_uncorrected;

	list<FaultDomain*> m_children;
	list<RepairScheme*> m_repairSchemes;
};


#endif /* FAULTDOMAIN_HH_ */
