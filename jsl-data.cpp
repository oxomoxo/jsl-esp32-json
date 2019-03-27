/*
	jsl-data.cpp

	This scource file is part of the jsl-esp32 project.

	Author: Lorenzo Pastrana
	Copyright © 2019 Lorenzo Pastrana

	This program is free software: you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by the
	Free Software Foundation, either version 3 of the License, or (at your
	option) any later version.

	This program is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
	or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
	for more details.

	You should have received a copy of the GNU General Public License along
	with this program. If not, see http://www.gnu.org/licenses/.

*/



#include <cstdlib>
#include <sstream>
#include <algorithm>


#define LOG_LOCAL_LEVEL ESP_LOG_NONE
// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
constexpr char DATA_LOGTAG[] = "DATA :";
#include <esp_log.h>

#include "jsl-data.h"



jsl_data::~jsl_data()
{
	clear();
}

std::string jsl_data::escape(const std::string& _str)
{
	std::string str = "";
	for(auto i = _str.begin(); i != _str.end(); ++ i)
	{
		switch(*i)
		{
		case '\t':
			str.push_back('\\');
			str.push_back('t');
			break;
		case '\n':
			str.push_back('\\');
			str.push_back('n');
			break;
		case '\r':
			str.push_back('\\');
			str.push_back('r');
			break;
		case '\f':
			str.push_back('\\');
			str.push_back('f');
			break;
		case '\b':
			str.push_back('\\');
			str.push_back('b');
			break;
		case '\0':
			str.push_back('\\');
			str.push_back('0');
			break;
		case '\'':
			str.push_back('\\');
			str.push_back('\'');
			break;
		case '\"':
			str.push_back('\\');
			str.push_back('"');
			break;
		case '\\':
			str.push_back('\\');
			str.push_back('\\');
			break;
		// case '/':
		// 	str.push_back('\\');
		// 	str.push_back('/');
		// 	break;
		default:
			str.push_back(*i);
		}
	}
	return str;
}

std::string jsl_data::to_string(bool _val)
{
	std::ostringstream s;
	s << ((_val == true) ? "true" : "false");
	return s.str();
}

std::string jsl_data::to_string(int32_t _val)
{
	std::ostringstream s;
	s << _val;
	return s.str();
}

std::string jsl_data::to_string(double _val)
{
	std::ostringstream s;
	s << _val;
	return s.str();
}

std::string jsl_data::to_string(const std::string& _val)
{
	return to_string(_val.c_str());
}

std::string jsl_data::to_string(const char* _val)
{
	return "\"" + escape(_val) + "\"";
}



jsl_data_scal::~jsl_data_scal()
{
	clear();
}

void jsl_data_scal::clear()
{
	clearStr();

	m_type = TYPE_NULL;
	m_scal.i = 0;

	jsl_data::clear();
}

void jsl_data_scal::fire()
{
	jsl_data_pool::fire(*this);
}

jsl_data_scal& jsl_data_scal::from_string(const char* _str)
{
	switch (m_type)
	{
	case TYPE_INT: {
		std::stringstream s(_str);
		s >> m_scal.i;
		break;
	}
	case TYPE_REAL: { /* -?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)? */
		std::stringstream s(_str);
		s >> m_scal.d;
		break;
	}
	case TYPE_BOOL:
		m_scal.b = (std::string("true") == _str) || (std::string("TRUE") == _str);
		break;
	case TYPE_STR:
		m_scal.s = _str;
		break;
	default:; // prevent compiler from complaining about other cases
	}
	return *this;
}

std::string jsl_data_scal::to_string() const
{
	switch (m_type)
	{
	case TYPE_NULL:
		return "null";
	case TYPE_INT:
		return jsl_data::to_string(m_scal.i);
	case TYPE_REAL:
		return jsl_data::to_string(m_scal.d);
	case TYPE_BOOL:
		return jsl_data::to_string(m_scal.b);
	case TYPE_STR:
		return jsl_data::to_string(m_scal.s);
	default:
		return "";
	}
}

const int32_t jsl_data_scal::empty_int = 0;
const double jsl_data_scal::empty_double = 0.;
const bool jsl_data_scal::empty_bool = false;
const std::string jsl_data_scal::empty_str;



jsl_data_dict::~jsl_data_dict()
{
	clear();
}

void jsl_data_dict::clear()
{
	m_container.clear(); //erase(m_container.begin(),m_container.end());

	jsl_data::clear();
}

void jsl_data_dict::fire()
{
	jsl_data_pool::fire(*this);
}

void jsl_data_dict::removeChild(const jsl_data& _child)
{
	for(auto found = m_container.begin(); found != m_container.end(); ++found)
	{
		if(found->second == &_child)
		{
			m_container.erase(found);
			return;
		}
	}
}

void jsl_data_dict::encode(std::ostream& _out, bool _pretty, std::string _tabs) const
{
	_out << "{";

	char nl = '\0', sp = '\0';

	if(_pretty)
	{
		sp = ' ';
		_tabs += '\t';
		_out << (nl = '\n');
	}

	for(auto i = m_container.begin(); i != m_container.end(); ++i)
	{
		_out << _tabs << '\"' << escape(i->first) << "\":" << sp;

		i->second->encode(_out,_pretty,_tabs);

		if(i != std::prev(m_container.end())) _out << ',';

		_out << nl;
	}

	if(_pretty) _tabs.pop_back();

	_out << _tabs << "}";

	if(_pretty && _tabs.size() == 0) _out << nl;
}



jsl_data_vect::~jsl_data_vect()
{
	clear();
}

void jsl_data_vect::clear()
{
	vect_t().swap(m_container);

	jsl_data::clear();
}

void jsl_data_vect::fire()
{
	jsl_data_pool::fire(*this);
}

void jsl_data_vect::removeChild(const jsl_data& _child)
{
	for(auto found = m_container.begin(); found != m_container.end(); ++found)
	{
		if(*found == &_child)
		{
			m_container.erase(found);
			return;
		}
	}
}

void jsl_data_vect::encode(std::ostream& _out, bool _pretty, std::string _tabs) const
{
	_out << "{";

	char nl = '\0';

	if(_pretty)
	{
		_tabs += '\t';
		_out << (nl = '\n');
	}

	for(auto i = m_container.begin(); i != m_container.end(); ++i)
	{
		_out << _tabs;

		(*i)->encode(_out,_pretty,_tabs);

		if(i != std::prev(m_container.end())) _out << ',';

		_out << nl;
	}

	if(_pretty) _tabs.pop_back();

	_out << _tabs << "]";

	if(_pretty && _tabs.size() == 0) _out << nl;
}



void jsl_data_pool::init(uint16_t _s, uint16_t _d, uint16_t _v)
{
	ESP_LOGI("DATA :","JSL_DATA_POOL::INIT");

	std::vector<jsl_data_scal>().swap(m_scals);
	std::vector<jsl_data_scal*>().swap(m_scals_for_hire);
	if(_s != 0)
	{
		m_scals.resize(_s);
		auto s = m_scals.begin();
		while(s != m_scals.end())
		{
			m_scals_for_hire.push_back(&(*s++));
		}
	}
	// ESP_LOGI(DATA_LOGTAG,"Scal pool_init %d => [%d:%d]",_s,m_scals.size(),m_scals_for_hire.size());

	std::vector<jsl_data_dict>().swap(m_dicts);
	std::vector<jsl_data_dict*>().swap(m_dicts_for_hire);
	if(_d != 0)
	{
		m_dicts.resize(_d);
		auto d = m_dicts.begin();
		while(d != m_dicts.end())
		{
			m_dicts_for_hire.push_back(&(*d++));
		}
	}
	// ESP_LOGI(DATA_LOGTAG,"Dict pool_init %d => [%d:%d]",_d,m_dicts.size(),m_dicts_for_hire.size());

	std::vector<jsl_data_vect>().swap(m_vects);
	std::vector<jsl_data_vect*>().swap(m_vects_for_hire);
	if(_v != 0)
	{
		m_vects.resize(_v);
		auto v = m_vects.begin();
		while(v != m_vects.end())
		{
			m_vects_for_hire.push_back(&(*v++));
		}
	}
	// ESP_LOGI(DATA_LOGTAG,"Vect pool_init %d => [%d:%d]",_v,m_vects.size(),m_vects_for_hire.size());
}

jsl_data_scal* jsl_data_pool::hire(int32_t _i)
{
	jsl_data_scal* data = hire_scal();
	(*data) = _i;
	return data;
}

jsl_data_scal* jsl_data_pool::hire(double _d)
{
	jsl_data_scal* data = hire_scal();
	(*data) = _d;
	return data;
}

jsl_data_scal* jsl_data_pool::hire(bool _b)
{
	jsl_data_scal* data = hire_scal();
	(*data) = _b;
	return data;
}

jsl_data_scal* jsl_data_pool::hire(const std::string& _s)
{
	jsl_data_scal* data = hire_scal();
	(*data) = _s;
	return data;
}

jsl_data_scal* jsl_data_pool::hire(const char* _s)
{
	jsl_data_scal* data = hire_scal();
	(*data) = _s;
	return data;
}

jsl_data_scal* jsl_data_pool::hire_scal()
{
	if(m_scals_for_hire.size() == 0)
	{
		return nullptr;
	}
	jsl_data_scal* data;
	data = m_scals_for_hire.back();
	m_scals_for_hire.pop_back();
	return data;
}

void jsl_data_pool::fire(jsl_data_scal& _data)
{
	_data.clear();
	if(std::find(m_scals_for_hire.begin(),m_scals_for_hire.end(),&_data) == m_scals_for_hire.end())
	{
		m_scals_for_hire.push_back(&_data);
	}
}

jsl_data_dict* jsl_data_pool::hire_dict()
{
	if(m_dicts_for_hire.size() == 0)
	{
		return nullptr;
	}
	jsl_data_dict* data;
	data = m_dicts_for_hire.back();
	m_dicts_for_hire.pop_back();
	return data;
}

void jsl_data_pool::fire(jsl_data_dict& _data)
{
	for(auto child = _data.begin(); child != _data.end(); ++child)
	{
		if(child->second != nullptr)
		{
			child->second->fire();
		}
	}
	_data.clear();
	if(std::find(m_dicts_for_hire.begin(),m_dicts_for_hire.end(),&_data) == m_dicts_for_hire.end())
	{
		m_dicts_for_hire.push_back(&_data);
	}
}

jsl_data_vect* jsl_data_pool::hire_vect()
{
	if(m_vects_for_hire.size() == 0)
	{
		return nullptr;
	}
	jsl_data_vect* data;
	data = m_vects_for_hire.back();
	m_vects_for_hire.pop_back();
	return data;
}

void jsl_data_pool::fire(jsl_data_vect& _data)
{
	for(auto child = _data.begin(); child != _data.end(); ++child)
	{
		if((*child) != nullptr)
		{
			(*child)->fire();
		}
	}
	_data.clear();
	if(std::find(m_vects_for_hire.begin(),m_vects_for_hire.end(),&_data) == m_vects_for_hire.end())
	{
		m_vects_for_hire.push_back(&_data);
	}
}

std::vector<jsl_data_scal>	jsl_data_pool::m_scals;
std::vector<jsl_data_scal*>	jsl_data_pool::m_scals_for_hire;
std::vector<jsl_data_dict>	jsl_data_pool::m_dicts;
std::vector<jsl_data_dict*>	jsl_data_pool::m_dicts_for_hire;
std::vector<jsl_data_vect>	jsl_data_pool::m_vects;
std::vector<jsl_data_vect*>	jsl_data_pool::m_vects_for_hire;

/*

void test_data()
{
	ESP_LOGI(LOGTAG, "Test DATA");

	ESP_LOGI(LOGTAG, "Data 2 : BOOL");
	jsl_data_scal& b = *jsl_data_pool::hire(true);
	b.encode(std::cout,true);// << "\n";

	ESP_LOGI(LOGTAG, "Data 1 : REAL");
	jsl_data_scal& d = *jsl_data_pool::hire(2.3);
	d.encode(std::cout,true);// << "\n";

	ESP_LOGI(LOGTAG, "Data 0 : INT");
	jsl_data_scal& i = *jsl_data_pool::hire(1);
	i.encode(std::cout,true);// << "\n";

	ESP_LOGI(LOGTAG, "Data 3 : STR");
	jsl_data_scal& s = *jsl_data_pool::hire("ok éô à");
	s.encode(std::cout,true);// << "\n";

	ESP_LOGI(LOGTAG, "Data 2 : Retype to STR");
	b = "b to s"; // re-type test
	b.encode(std::cout,true);// << "\n";

	ESP_LOGI(LOGTAG, "Data 2 : Retype to BOOL");
	b = true; // re-type test
	b.encode(std::cout,true);// << "\n";

	ESP_LOGI(LOGTAG, "Data 4 : DICT");
	// jsl_data_dict& dict = *((jsl_data_dict*)jsl_data_pool::hire(jsl_data::TYPE_DICT));
	jsl_data_dict& dict = *jsl_data_pool::hire_dict();

	dict.set_prop("i",i);
	// dict.set_prop("d",d);
	dict.set_prop("b",b);
	dict.set_prop("s",s);

	ESP_LOGI(LOGTAG, "Data 5 : VECT");
	// jsl_data_vect& vect = *((jsl_data_vect*)jsl_data_pool::hire(jsl_data::TYPE_VECT));
	jsl_data_vect& vect = *jsl_data_pool::hire_vect();

	// vect.push_back(i);
	vect.push_back(d);
	// vect.push_back(b);
	// vect.push_back(s);

	ESP_LOGI(LOGTAG, "Data 2 : From STR");
	d.from_string("2.3456");

	ESP_LOGI(LOGTAG, "Data 4 : VECT Child");
	dict.set_prop("v",vect);
	ESP_LOGI(LOGTAG, "Data 4 : ENCODE");
	dict.encode(std::cout,true);// << "\n\n";

	// vect.push_back(&dict);
	// vect.encode(std::cout,true);

	ESP_LOGI(LOGTAG, "Data FIRE");

	jsl_data_pool::fire(i);
	jsl_data_pool::fire(d);
	jsl_data_pool::fire(b);
	jsl_data_pool::fire(s);
	jsl_data_pool::fire(dict);
	jsl_data_pool::fire(vect);
}

*/
