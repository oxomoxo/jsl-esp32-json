/*
	jsl-parser.cpp

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



#define PARSER_LOGTAG "PARSER :"
#include <esp_log.h>

// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <iostream>
#include "json/jsl-parser.h"

jsl_data_dict* jsl_parser::parse()
{
	m_src.seekg(0);
	return eat_dict();
}

jsl_data_dict* jsl_parser::eat_dict()
{
	if(m_src.peek() != '{')
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : wrong init char [%c]",m_src.peek());
		return NULL;
	}

	m_src.get();

	jsl_data_dict* dict = jsl_data_pool::hire_dict();
	if(dict == NULL)
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : hire dict fail");
		return NULL;
	}

	std::string pname;
	jsl_data* pvalue = NULL;

	while(!m_src.eof())
	{
		if(eat_space())
		{
			ESP_LOGE(PARSER_LOGTAG, "Error : eat_space unexpected EOF");
			goto abort;
		} // EOF

		switch(m_src.peek())
		{
		case '"': // prop name
			if(!pname.empty())
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : prop-name already there");
				goto abort;
			}
			if(scan_str(pname))
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : scan_str unexpected EOF");
				goto abort;
			} // EOF
			break;
		case ':': // prop val
			if(pname.empty())
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : prop-name was not there");
				goto abort;
			}
			m_src.get();
			pvalue = eat_value();
			if(!pvalue)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : eat_value fail");
				goto abort;
			}
			dict->set_prop(pname,*pvalue);
			pvalue = NULL;
			pname = "";
			break;
		case ',': // next
			if(!pname.empty())
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : ");
				goto abort;
			}
			m_src.get();
			break;
		case '}': // end
			m_src.get();
			return dict;
		default:
			ESP_LOGE(PARSER_LOGTAG, "unexpected char [%c]",m_src.peek());
			goto abort; // invalid src
		}
	}

	ESP_LOGE(PARSER_LOGTAG, "Error : eat_dict unexpected EOF");
	return NULL; // EOF

abort:

	ESP_LOGE(PARSER_LOGTAG, "Error : eat_dict aborted");
	jsl_data_pool::fire(*dict);
	return NULL; // aborted
}

jsl_data_vect* jsl_parser::eat_vect()
{
	if(m_src.peek() != '[')
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : wrong init char [%c]",m_src.peek());
		return NULL;
	}

	m_src.get();

	jsl_data_vect* vect = jsl_data_pool::hire_vect();
	if(vect == NULL)
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : hire vect fail");
		return NULL;
	}

	jsl_data* pvalue = NULL;

	while(!m_src.eof())
	{
		pvalue = eat_value();
		if(!pvalue)
		{
			ESP_LOGE(PARSER_LOGTAG, "Error : eat_value fail");
			goto abort;
		}

		vect->push_back(*pvalue);
		pvalue = NULL;

		if(eat_space())
		{
			ESP_LOGE(PARSER_LOGTAG, "Error : eat_space unexpected EOF");
			goto abort;
		} // EOF

		switch(m_src.peek())
		{
		case ',': // next
			m_src.get();
			break;
		case ']': // end
			m_src.get();
			return vect;
		default:
			ESP_LOGE(PARSER_LOGTAG, "unexpected char [%c]",m_src.peek());
			goto abort; // invalid src
		}

	}

	ESP_LOGE(PARSER_LOGTAG, "Error : eat_vect unexpected EOF");
	return NULL; // EOF

abort:

	ESP_LOGE(PARSER_LOGTAG, "Error : eat_vect aborted");
	jsl_data_pool::fire(*vect);
	return NULL; // aborted
}

jsl_data* jsl_parser::eat_value()
{
	if(eat_space())
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : no food");
		return NULL;
	} // EOF

	switch(m_src.peek())
	{
	case 'n': // null
		return eat_null();
	case 'f': // null
		return eat_false();
	case 't': // null
		return eat_true();
	case '{': // dict
		return eat_dict();
	case '[': // vect
		return eat_vect();
	case '"': // string
		return eat_str();
	// case '\'': // char
	// 	return eat_chr();
	case '0': // number
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
	case '.':
		return eat_num();

	default: return NULL; // invalid src
	}
}

jsl_data_scal* jsl_parser::eat_null()
{
	if(eat_space())
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : no food");
		return NULL;
	} // EOF

	char buf[5] = {};
	m_src.get(buf,5);

	if(

		buf[0] == 'n' &&
		buf[1] == 'u' &&
		buf[2] == 'l' &&
		buf[3] == 'l'

	){

		return jsl_data_pool::hire_scal(); // defaults to NULL

	}

	m_src.seekg(-4,std::istream::cur);

	ESP_LOGE(PARSER_LOGTAG, "Error : wrong null chars [%s]",buf);
	return NULL;

}

jsl_data_scal* jsl_parser::eat_false()
{
	if(eat_space())
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : no food");
		return NULL;
	} // EOF

	char buf[6] = {};
	m_src.get(buf,6);

	if(

		buf[0] == 'f' &&
		buf[1] == 'a' &&
		buf[2] == 'l' &&
		buf[3] == 's' &&
		buf[4] == 'e'

	){

		return jsl_data_pool::hire(1 == 2); // false

	}

	m_src.seekg(-5,std::istream::cur);

	ESP_LOGE(PARSER_LOGTAG, "Error : wrong false chars [%s]",buf);
	return NULL;

}

jsl_data_scal* jsl_parser::eat_true()
{
	if(eat_space())
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : no food");
		return NULL;
	} // EOF

	char buf[5] = {};
	m_src.get(buf,5);

	if(

		buf[0] == 't' &&
		buf[1] == 'r' &&
		buf[2] == 'u' &&
		buf[3] == 'e'

	){

		return jsl_data_pool::hire(1 == 1); // true

	}

	m_src.seekg(-4,std::istream::cur);

	ESP_LOGE(PARSER_LOGTAG, "Error : wrong true chars [%s]",buf);
	return NULL;

}

jsl_data_scal* jsl_parser::eat_num()
{
	if(eat_space())
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : no food");
		return NULL;
	} // EOF

	typedef enum
	{
		STATE_START,
		STATE_SIGN,
		STATE_ZERO,
		STATE_INTG,
		STATE_REALS,
		STATE_REAL,
		STATE_EXPOS,
		STATE_EXPO
	} num_state_t;

	std::string str;
	num_state_t st = STATE_START;

	while(!m_src.eof())
	{
		uint8_t p = m_src.peek();
		if(is_space(p) || p == ',') break; // end of number

		// validate number
		switch(p)
		{
		case '-':
			// allowed source state ?
			if(st != STATE_START && st != STATE_EXPOS)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [-] : %d",st);
				return NULL;
			}
			// set new state depending on source
			if(st == STATE_EXPOS) st = STATE_EXPO;
			else st = STATE_SIGN;
			break;
		case '.':
			// allowed source state ?
			if(st != STATE_ZERO && st != STATE_INTG)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [.] : %d",st);
				return NULL;
			}
			// set new state depending on source
			st = STATE_REALS;
			break;
		case 'E':
		case 'e':
			// allowed source state ?
			if(st != STATE_INTG && st != STATE_REAL)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [E] : %d",st);
				return NULL;
			}
			// set new state depending on source
			st = STATE_EXPOS;
			break;
		case '+':
			// allowed source state ?
			if(st != STATE_EXPOS)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [+] : %d",st);
				return NULL;
			}
			// set new state depending on source
			st = STATE_EXPO;
			break;
		case '0':
			// allowed source state ?
			if(st != STATE_START && st != STATE_SIGN && st != STATE_INTG && st != STATE_REALS && st != STATE_REAL && st != STATE_EXPOS && st != STATE_EXPO)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [0] : %d",st);
				return NULL;
			}
			// set new state depending on source
			if(st == STATE_START || st == STATE_SIGN) st = STATE_ZERO;
			else if(st == STATE_REALS) st = STATE_REAL;
			else if(st == STATE_EXPOS) st = STATE_EXPO;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			// allowed source state ?
			if(st != STATE_START && st != STATE_SIGN && st != STATE_INTG && st != STATE_REALS && st != STATE_REAL && st != STATE_EXPOS && st != STATE_EXPO)
			{
				ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [digit] : %d",st);
				return NULL;
			}
			// set new state depending on source
			if(st == STATE_START || st == STATE_SIGN) st = STATE_INTG;
			else if(st == STATE_REALS) st = STATE_REAL;
			else if(st == STATE_EXPOS) st = STATE_EXPO;
			break;
		default: return NULL; // invalid src
		}

		// eat stream
		str.push_back(m_src.get());
	}

	if(st != STATE_ZERO && st != STATE_INTG && st != STATE_REAL && st != STATE_EXPO)
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : illegal num state [end] : %d",st);
		return NULL;
	}

	jsl_data_scal* scal = NULL;

	if(st == STATE_INTG)
	{
		int32_t num = 0;
		scal = jsl_data_pool::hire(num);
		if(scal != NULL) scal->from_string(str);
	}
	else
	{
		double num = 0;
		scal = jsl_data_pool::hire(num);
		if(scal != NULL) scal->from_string(str);
	}

	return scal;
}

jsl_data_scal* jsl_parser::eat_str()
{
	std::string str;
	if(scan_str(str))
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : unexpected EOF");
		return NULL;
	} // EOF

	return jsl_data_pool::hire(str);
}

bool jsl_parser::scan_str(std::string& _str)
{
	if(m_src.peek() != '"')
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : wrong init char [%c]",m_src.peek());
		return NULL;
	}

	m_src.get();

	while(!m_src.eof())
	{
		switch(m_src.peek())
		{
		case '"': // end
			m_src.get();
			return false; // Not EOF
		case '\\': // unescape
			if(unescape(_str)) return true;
			break;
		default:
			_str.push_back(m_src.get());
		}
	}

	return true; // unexpected EOF
}

bool jsl_parser::unescape(std::string& _str)
{
	if(m_src.peek() != '\\')
	{
		ESP_LOGE(PARSER_LOGTAG, "Error : wrong init char [%c]",m_src.peek());
		return true;
	}

	m_src.get();

	switch(m_src.peek())
	{
	case '"':
	case '\\':
	case '/':
		_str.push_back(m_src.get());
		break;
	case 'f':
		_str.push_back('\f');
		m_src.get();
		break;
	case 'b':
		_str.push_back('\b');
		m_src.get();
		break;
	case 'n':
		_str.push_back('\n');
		m_src.get();
		break;
	case 'r':
		_str.push_back('\r');
		m_src.get();
		break;
	case 't':
		_str.push_back('\t');
		m_src.get();
		break;
	case 'u':
		m_src.get();

		char buf[5] = {};
		m_src.get(buf,4);
		std::string::value_type ch = std::strtol(buf,NULL,16);
		utf8_str(ch,_str);
		break;
	}

	return false;
}

void jsl_parser::utf8_str(uint32_t _char, std::string& _str)
{
	char buf[5] = {};
	if (_char <= 0x7F)
	{
		buf[0] = static_cast<char>(_char);
	}
	else if (_char <= 0x7FF)
	{
		buf[1] = 0x80 | (_char & 0x3F);
		_char >>= 6;
		buf[0] = 0xC0 | _char;
	}
	else if (_char <= 0xFFFF)
	{
		buf[2] = 0x80 | (_char & 0x3F);
		_char >>= 6;
		buf[1] = 0x80 | (_char & 0x3F);
		_char >>= 6;
		buf[0] = 0xE0 | _char;
	}
	else
	{
		buf[3] = 0x80 | (_char & 0x3F);
		_char >>= 6;
		buf[2] = 0x80 | (_char & 0x3F);
		_char >>= 6;
		buf[1] = 0x80 | (_char & 0x3F);
		_char >>= 6;
		buf[0] = 0xF0 | _char;
	}
	_str += buf;
}

bool jsl_parser::is_space(uint8_t _c)
{
	return
		(_c == '\f') |
		(_c == '\b') |
		(_c == '\n') |
		(_c == '\r') |
		(_c == '\t') |
		(_c == ' ');
}

bool jsl_parser::eat_space()
{
	while(is_space(m_src.peek()))
	{
		m_src.get();
		if(m_src.eof()) return true; // EOF
	}
	return false; // not EOF
}

