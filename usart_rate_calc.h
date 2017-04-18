/** Copyright 2008-2014 Josh Pieper, jjp@pobox.com.  All rights reserved. */

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef USART_RATE_CALC_H
#define USART_RATE_CALC_H

// The following are macros to calculate the best UBRR and doubling
// value given a clock frequency and desired baud rate.  Use
// UBR_FROM_BAUD and DOUBLE_FROM_BAUD.  It is wise to cause compilation
// to fail if BAUD_ERROR_TOO_HIGH is true, since communication will
// be unreliable
//
// freq = frequency the chip runs at
// baud = desired baud rate
//

#define TRUNCATE_FIXED(val) (((val)/100)*100)
#define ROUND_FIXED(val) ((((val)-TRUNCATE_FIXED(val))>50)?(TRUNCATE_FIXED(val)+100):(TRUNCATE_FIXED(val)))
#define PROPOSED_UBRR(freq, baud, mult) (ROUND_FIXED(((freq*100)/((mult)*(baud)))-100)/100)
#define ACTUAL_BAUD(freq, baud, mult) ((freq)/(((PROPOSED_UBRR(freq,baud,mult))+1)*(mult)))
#define BAUD_ERROR(freq, baud, mult) (((ACTUAL_BAUD(freq, baud, mult)-baud)*1000)/baud)
#define ABS_BAUD_ERROR(freq, baud, mult) ((BAUD_ERROR(freq, baud, mult)<0)?(-BAUD_ERROR(freq, baud, mult)):(BAUD_ERROR(freq,baud,mult)))


#define BAUD_ERROR_TOO_HIGH(freq, baud) (ABS_BAUD_ERROR(freq,baud,8)>5 && ABS_BAUD_ERROR(freq,baud,16)>10)
#define UBR_FROM_BAUD(freq,baud) ((ABS_BAUD_ERROR((uint32_t)freq,(uint32_t)baud,16)<=5)?(PROPOSED_UBRR((uint32_t)freq, (uint32_t)baud, 16)):(PROPOSED_UBRR((uint32_t)freq,(uint32_t)baud, 8)))
#define DOUBLE_FROM_BAUD(freq, baud) (ABS_BAUD_ERROR((uint32_t)freq,(uint32_t)baud,16)>5)

#endif
