/*
 * FAULTSIM: A Fast, Configurable Memory Resilience Simulator
 * (c) 2013-2015 Advanced Micro Devices, Inc.
 *
 */

#ifndef BCHREPAIR_HH_
#define BCHREPAIR_HH_

#include "RepairScheme.hh"

class BCHRepair : public RepairScheme
{
public:
	// need to know how wide the devices are to determine which bits fall into one codeword
	// across all the chips
	BCHRepair( string name, int n_correct,int n_detect, uint64_t deviceBitWidth );
	uint64_t fill_repl ( FaultDomain *fd );
	void repair( FaultDomain *fd, uint64_t &n_undetectable, uint64_t &n_uncorrectable );

	void printStats( void );
	void resetStats( void );
	void clear_counters ( void );

private:
	uint64_t m_n_correct, m_n_detect, m_bitwidth;
	uint64_t counter_prev, counter_now;
};


#endif /* BCHREPAIR_HH_ */
