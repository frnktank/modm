// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2011, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_STM32__PLL_HPP
#define XPCC_STM32__PLL_HPP

#include "../common/common_clock.hpp"

using namespace xpcc::clock;

namespace xpcc
{
	namespace stm32
	{

		/*
		 *
		 * For Stm32F2 VCOOutputMinimum needs to be MHz64
		 * For Stm32F4 VCOOutputMinimum needs to be MHz192
		 */
		template<int64_t VCOOutputMinimum, int64_t InputFrequency,
					int64_t SystemFrequency, int64_t USBFrequency>
		class
		Stm32F2F4PllSettings
		{
		private:
			// Processor Specific Values
			static constexpr int64_t VCOInputMin  = MHz1;
			static constexpr int64_t VCOInputMax  = MHz2;
			static constexpr int64_t VCOOutputMin =  VCOOutputMinimum;
			static constexpr int64_t VCOOutputMax = MHz432;
			// Pll Constant Range
			static constexpr int64_t Mmin =   2;
			static constexpr int64_t Mmax =  63;
			static constexpr int64_t Nmin =  64;
			static constexpr int64_t Nmax = 432;
			static constexpr int64_t Qmin =   2;
			static constexpr int64_t Qmax =  15;


//------------------------------- PllM -----------------------------------------
			static constexpr int64_t
			checkM(int64_t m)
			{
				return ((InputFrequency / m) <= VCOInputMax &&
						(InputFrequency / m) >= VCOInputMin &&
						(calculatePllN(m) >= 0));
			}

			static constexpr int64_t
			calculatePllM(int64_t m = Mmin)
			{
				return checkM(m)? m : ((m < Mmax)? calculatePllM(m + 1) : -1);
			}

//------------------------------- PllN -----------------------------------------
			static constexpr int64_t
			checkN(int64_t m, int64_t n)
			{
				return ((InputFrequency / m * n) <= VCOOutputMax &&
						(InputFrequency / m * n) >= VCOOutputMin &&
						(calculatePllP(m, n) >= 0));// &&
						// (calculatePllQ(m, n) >= 0));
			}

			static constexpr int64_t
			calculatePllN(int64_t m, int64_t n = Nmax)
			{
				return checkN(m, n)? n : ((n > Nmin)? calculatePllN(m, n - 1) : -1);
			}

//------------------------------- PllP -----------------------------------------
			static constexpr int64_t
			pllP(int64_t m, int64_t n)
			{
				// SystemFrequency = InputFrequency / PllM * PllN / PllP
				// => PllP = InputFrequency / PllM * PllN / SystemFrequency
				return InputFrequency * n / m / SystemFrequency;
			}

			static constexpr int64_t
			checkP(int64_t m, int64_t n, int64_t p)
			{
				// SystemFrequency = InputFrequency / PllM * PllN / PllP
				return ((p == 2 || p == 4 || p == 6 || p == 8) &&
						(InputFrequency / m * n / p) == SystemFrequency);
			}

			static constexpr int64_t
			calculatePllP(int64_t m, int64_t n)
			{
				return checkP(m, n, pllP(m, n))? pllP(m, n) : -1;
			}

//------------------------------- PllQ -----------------------------------------
			static constexpr int64_t
			pllQ(int64_t m, int64_t n)
			{
				// USBFrequency = InputFrequency / PllM * PllN / PllQ
				// => PllQ = InputFrequency / PllM * PllN / USBFrequency
				return InputFrequency * n / m / USBFrequency;
			}

			static constexpr int64_t
			checkQ(int64_t m, int64_t n, int64_t q)
			{
				// USBFrequency = InputFrequency / PllM * PllN / PllQ
				return (q >= Qmin && q <= Qmax &&
						(InputFrequency / m * n / q) == USBFrequency);
			}

			static constexpr int64_t
			calculatePllQ(int64_t m, int64_t n)
			{
				return checkQ(m, n, pllQ(m, n))? pllQ(m, n) : -1;
			}


			// Internal Pll Constants Representation
			static constexpr int64_t _PllM = calculatePllM(Mmin);		// TODO: remove default
			static constexpr int64_t _PllN = calculatePllN(_PllM, Nmax);	// TODO: remove default
			static constexpr int64_t _PllP = calculatePllP(_PllM, _PllN);
			static constexpr int64_t _PllQ = calculatePllQ(_PllM, _PllN);
		public:
			// Pll Constants casted to the correct datatype
			static constexpr uint8_t  PllM = (_PllM > 0)? static_cast<uint8_t>(_PllM)  : 0xff;
			static constexpr uint16_t PllN = (_PllN > 0)? static_cast<uint16_t>(_PllN) : 0xffff;
			static constexpr uint8_t  PllP = (_PllP > 0)? static_cast<uint8_t>(_PllP)  : 0xff;
			static constexpr uint8_t  PllQ = (_PllQ > 0)? static_cast<uint8_t>(_PllQ)  : 0xff;
			// Resulting Frequencies
			static constexpr int64_t VCOInput    = InputFrequency / PllM;
			static constexpr int64_t VCOOutput   = VCOInput * PllN;
			static constexpr int64_t SystemClock = VCOOutput / PllP;
			static constexpr int64_t USBClock    = VCOOutput / PllQ;
		private:
			// Static Asserts
			// Check Ranges
			static_assert(PllM >= Mmin && PllM <= Mmax, "PllM is out of range!");
			static_assert(PllN >= Nmin && PllN <= Nmax, "PllQ is out of range!");
			static_assert(PllQ >= Qmin && PllP <= Qmax, "PllQ is out of range!");
			static_assert(PllP == 2 || PllP == 4 || PllP == 6 || PllP == 8,
							"PllP is an invalid value (possible values are 2,4,6,8)!");
			// Check that VCOInput is between 1 and 2 MHz
			static_assert(VCOInput >= MHz1 && VCOInput <= MHz2,
				"VCOInput Frequency needs to be between 1MHz and 2MHz! "
				"'VCOInput = InputFrequency / PllM' "
				"Probably no good value for PllM could be found. "
				"Please consult your STM32's Reference Manual page.");
			// Check that VCOOutput is between 192 and 432 MHz
			static_assert(VCOOutput >= VCOOutputMin && VCOOutput <= VCOOutputMax,
				"VCOOutput Frequency needs to be in range! "
				"'VCOOutput = VCOInput * PllN' "
				"Probably no good value for PllN could be found. "
				"Please consult your STM32's Reference Manual page.");
			// Check if desired clock frequency is met
			static_assert(SystemClock == SystemFrequency,
				"Desired Output Frequency could not be met."
				"Please consult your STM32's Reference Manual page.");
			// Check if desired usb frequency is met
			static_assert(USBClock == USBFrequency,
				"Desired Output Frequency could not be met."
				"Please consult your STM32's Reference Manual page.");
		};
	}
}

#endif	//  XPCC_STM32__PLL_HPP
