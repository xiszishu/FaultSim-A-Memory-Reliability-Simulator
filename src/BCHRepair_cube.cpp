/*
Copyright (c) 2015, Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BCHRepair_cube.hh"
#include "DRAMDomain.hh"
#include "Settings.hh"
<<<<<<< HEAD
#include <iostream>
=======
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7

extern struct Settings settings;

BCHRepair_cube::BCHRepair_cube( string name, int n_correct, int n_detect, uint64_t data_block_bits ) : RepairScheme( name )
, m_n_correct(n_correct)
, m_n_detect(n_detect)
, m_bitwidth(data_block_bits)
{
	counter_prev=0;
	counter_now=0;
	m_log_block_bits = log2( data_block_bits );
}

void BCHRepair_cube::repair( FaultDomain *fd, uint64_t &n_undetectable, uint64_t &n_uncorrectable )
{
	n_undetectable = n_uncorrectable = 0;

	// Repair up to N bit faults in a single block
	uint bit_shift=0;
	uint loopcount_locations=0;
	uint ii=0;
	list<FaultDomain*> *pChips = fd->getChildren();
	//assert( pChips->size() == (m_n_repair * 18) );

	list<FaultDomain*>::iterator it0, it1;
	for(it1 =pChips->begin(); it1 !=pChips->end(); it1++)
	{
		DRAMDomain *pDRAM1 = dynamic_cast<DRAMDomain*>((*it1));
		list<FaultRange*> *pRange3 = pDRAM1->getRanges();
		list<FaultRange*>::iterator itRange3;
		for(itRange3 = pRange3->begin(); itRange3 !=pRange3->end(); itRange3++)
		{
			FaultRange *fr1 =(*itRange3);
			fr1->touched=0;
		}
	}

	// Take each chip in turn.  For every fault range in a chip, see which neighbors intersect it's ECC block(s).
	// Count the failed bits in each ECC block.
	for( it0 = pChips->begin(); it0 != pChips->end(); it0++ )
	{
		DRAMDomain *pDRAM0 = dynamic_cast<DRAMDomain*>((*it0));
		list<FaultRange*> *pRange0 = pDRAM0->getRanges();

		list<FaultRange*>::iterator itRange0;
		for( itRange0 = pRange0->begin(); itRange0 != pRange0->end(); itRange0++ )
		{
			FaultRange *frOrg = (*itRange0); // The pointer to the fault location
			FaultRange frTemp = *(*itRange0); //This is a fault location of a chip

			uint32_t n_intersections = 0;
			
			if(frTemp.touched < frTemp.max_faults)
			{
				if( settings.debug ) {
<<<<<<< HEAD
            std::cout << m_name << ": outer " << frTemp.toString() << "\n";
=======
					cout << m_name << ": outer " << frTemp.toString() << "\n";
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7
				}

				bit_shift=m_log_block_bits;	//ECC every 64 byte i.e 512 bit granularity
				frTemp.fAddr = frTemp.fAddr >> bit_shift;
				frTemp.fAddr = frTemp.fAddr << bit_shift;
				frTemp.fWildMask = frTemp.fWildMask >> bit_shift;
				frTemp.fWildMask = frTemp.fWildMask << bit_shift;
				loopcount_locations = 1 << bit_shift; // This gives me the number of loops for the addresses near the fault range to iterate

				for(ii=0;ii<loopcount_locations;ii++)
				{
					DRAMDomain *pDRAM1 = dynamic_cast<DRAMDomain*>((*it0));
					list<FaultRange*> *pRange1 = pDRAM1->getRanges();
					list<FaultRange*>::iterator itRange1;
					for( itRange1 = pRange1->begin(); itRange1 != pRange1->end(); itRange1++ )
					{
						FaultRange *fr1 = (*itRange1);

						if( settings.debug ) {
<<<<<<< HEAD
                std::cout << m_name << ": inner " << fr1->toString() << " bit " << ii << "\n";
=======
							cout << m_name << ": inner " << fr1->toString() << " bit " << ii << "\n";
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7
						}

						if( fr1->touched < fr1->max_faults)
						{
							if(frTemp.intersects(fr1)) {
<<<<<<< HEAD
                  if( settings.debug ) std::cout << m_name << ": INTERSECT " << n_intersections << "\n";
=======
								if( settings.debug ) cout << m_name << ": INTERSECT " << n_intersections << "\n";
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7

								n_intersections++;

								// There was a failed bit in at least one row of the FaultRange of interest.
								// We now only care about further intersections that are in the overlapping
								// rows of the two ranges.  Narrow down the search to only those rows in common
								// to both FaultRanges.  This is achieved by;
								// 1) Set upper mask bits to zero if they are not wild in range under test
								// 2) For those wild bits that we cleared, use the specific address bit value
								uint64_t fr1_fAddr_upper = (fr1->fAddr >> bit_shift) << bit_shift;
								uint64_t frTemp_fAddr_lower = (frTemp.fAddr & ((0x1 << bit_shift)-1) );

								uint64_t old_wild_mask = frTemp.fWildMask;
								frTemp.fWildMask &= fr1->fWildMask;
								uint64_t changed_wild_bits = old_wild_mask ^ frTemp.fWildMask;
								frTemp.fAddr = (fr1_fAddr_upper & changed_wild_bits) | (frTemp.fAddr & (~changed_wild_bits)) | frTemp_fAddr_lower;

								// immediately move on to the next location
								break;
							} else {
<<<<<<< HEAD
                  if( settings.debug ) std::cout << m_name << ": NONE " << n_intersections << "\n";
=======
								if( settings.debug ) cout << m_name << ": NONE " << n_intersections << "\n";
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7
							}
						}
					}
					frTemp.fAddr = frTemp.fAddr + 1;
				}

				// For this algorithm, one intersection with the bit being tested actually means one
				// faulty bit in the
				if(n_intersections <= m_n_correct)
				{
					// correctable
				}
				if(n_intersections > m_n_correct)
				{
					n_uncorrectable += (n_intersections - m_n_correct);
					frOrg->transient_remove = false;
					if( !settings.continue_running ) return;
				}
				if(n_intersections >= m_n_detect)
				{
					n_undetectable += (n_intersections - m_n_detect);
				}
			}
		}
	}
}

uint64_t BCHRepair_cube::fill_repl(FaultDomain *fd)
{
	return 0;
}
void BCHRepair_cube::printStats( void )
{
	RepairScheme::printStats();
}

void BCHRepair_cube::clear_counters(void)
{
	counter_prev=0;
	counter_now=0;
}

void BCHRepair_cube::resetStats( void )
{
	RepairScheme::resetStats();
}
