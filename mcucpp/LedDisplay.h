#ifndef LED_HPP
#define LED_HPP

//*****************************************************************************
//
// Title		: C++ Led display
// Author		: Konstantin Chizhov
// Date			: 2010
// Target MCU	: Atmel mega AVR, TINY avr AND Xmega Series
//
//       This code is distributed under the GNU Public License
//       which can be found at http://www.gnu.org/licenses/gpl.txt
//
// As a special exception, you may use this file as part of a free software
// library without restriction. Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License. This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.
//*****************************************************************************

#include "util.h"
#include "static_assert.h"
template<
	class A, 
	class B, 
	class C, 
	class D, 
	class E, 
	class F,
	class G,
	class DP
	>
class Segments
{
public:
	void Write(uint8_t value)
	{
		a.Set(value & 1);
		b.Set(value & 2);
		c.Set(value & 4);
		d.Set(value & 8);
		e.Set(value & 16);
		f.Set(value & 32);
		g.Set(value & 64);
		dp.Set(value & 128);
	}
private:
	A a; B b; C c; D d; E e; F f; G g; DP dp;
};

template<
	class A, 
	class B, 
	class C, 
	class D, 
	class E, 
	class F,
	class G,
	class DP
	>
class SegmentsInv
{
public:
	void Write(uint8_t value)
	{
		value=~value;
		a.Set(value & 1);
		b.Set(value & 2);
		c.Set(value & 4);
		d.Set(value & 8);
		e.Set(value & 16);
		f.Set(value & 32);
		g.Set(value & 64);
		dp.Set(value & 128);
	}
private:
	A a; B b; C c; D d; E e; F f; G g; DP dp;
};

template<class PORT>
class PortSegments
{
public:
	void Write(uint8_t value)
	{
		PORT::dir() = 0xff;
		PORT::Write(value);
	}
};

template<class PORT>
class PortSegmentsInv
{
public:
	void Write(uint8_t value)
	{
		PORT::dir() = 0xff;
		PORT::Write(~value);
	}
};

template<class PORT, uint8_t NUM_DIDGITS>
class CommonsPortH // active level - hi
{
	BOOST_STATIC_ASSERT(NUM_DIDGITS<=8);
public:
	
	enum{NumDidgits = NUM_DIDGITS, PortMask = Pow<2, NUM_DIDGITS>::value - 1};
	void Set(uint8_t n)
	{
		PORT::dir() |= (uint8_t)(PortMask);
		PORT::data() = (PORT::data() & (uint8_t)(~PortMask)) | (uint8_t)(1 << n);
	}
};

template<class PORT, uint8_t NUM_DIDGITS>
class CommonsPortL // active level - low
{
	BOOST_STATIC_ASSERT(NUM_DIDGITS<=8);
public:
	enum{NumDidgits = NUM_DIDGITS, PortMask = Pow<2, NUM_DIDGITS>::value - 1};
	void Set(uint8_t n)
	{
		PORT::dir() |= (uint8_t)(PortMask);
		PORT::data() = (PORT::data() & (uint8_t)(~PortMask)) | (uint8_t)~(1 << n);
	}
};


class LedMapTable
{
public:
	uint8_t Map(uint8_t bcd)
	{
		return pgm_read_byte(PSTR("\x3f\x06\x5b\x4f\x66\x6d\x7d\x7\x7f\x6f\x77\x7c\x39\x5e\x79\x71")+bcd);
	}

	uint8_t Minus(){return 0x40;}
	uint8_t Empty(){return 0x0;}
	 
};

template<class SEGMENTS, class COMMONS, class MAP_TABLE=LedMapTable>
class LedDisplay
{
public:
	LedDisplay()
	{
		position = 0;
	}

	void WriteDec(int16_t value)
	{
		int16_t i= 0; 
		uint8_t minus=0;
	    if (value < 0)  
		{
	        value = -value;
			minus = 1;
		}
		div_t res;
	    do {
			res = div(value, 10);
	        _value[NumDidgits - 1 - i++] = _table.Map(res.rem);
			if(i>=NumDidgits)return;
	    } while ((value = res.quot) > 0);
	    if (minus)
	        _value[NumDidgits - 1 - i++] = _table.Minus();
		while(i < NumDidgits)
			_value[NumDidgits - 1 - i++] = _table.Empty();
	}

	/*void WriteDec(int32_t value)
	{
		int16_t i= 0; 
		uint8_t minus=0;
	    if (value < 0)  
		{
	        value = -value;
			minus = 1;
		}
		div_t res;
	    do {
			res = div(value, 10);
	        _value[NumDidgits - 1 - i++] = _table.Map(res.rem);
			if(i>=NumDidgits)return;
	    } while ((value = res.quot) > 0);
	    if (minus)
	        _value[i++] = _table.Minus();
		while(i++ < NumDidgits)
			_value[NumDidgits - 1 - i] = _table.Empty();
	}*/

	void WriteHex(uint16_t value)
	{
		for(uint8_t i = 0; i < NumDidgits; i++)
		{
			uint8_t rem = value & 0x0f;
			_value[NumDidgits-i-1] = _table.Map(rem);
			value >>= 4;
			if(value==0 && rem==0)
			{
				_value[NumDidgits-i-1] = 0;
			}	
		}
	}

	void WriteHex(uint32_t value)
	{
		for(uint8_t i = 0; i < NumDidgits; i++)
		{
			uint8_t rem = value & 0x0f;
			_value[NumDidgits-i-1] = _table.Map(rem);
			value >>= 4;
			if(value==0 && rem==0)
			{
				_value[NumDidgits-i-1] = 0;
			}	
		}
	}

	void Update()
	{
		_commons.Set(position);
		_segments.Write(_value[position++]);
		if(position >= NumDidgits)
			position = 0;
	}
	
protected:
	uint8_t position;
	enum{NumDidgits = (COMMONS::NumDidgits)};
	uint8_t _value[NumDidgits];
	SEGMENTS _segments;
	COMMONS _commons;
	MAP_TABLE _table;
};

#endif