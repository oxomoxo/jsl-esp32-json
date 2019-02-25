/*
	jsl-parser.h

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



#ifndef JSL_PARSER_H
#define JSL_PARSER_H

#include <string>

#include "jsl-data.h"



class jsl_parser
{
public:

	typedef const std::string& src_t;
	typedef std::string::const_iterator src_i;

	jsl_parser(const std::string& _src) : m_src(_src) {}

	jsl_data_dict* parse();

protected:

	jsl_data_dict* eat_dict();
	jsl_data_vect* eat_vect();

	jsl_data* eat_value();

	jsl_data_scal* eat_null();
	jsl_data_scal* eat_false();
	jsl_data_scal* eat_true();
	jsl_data_scal* eat_num();
	jsl_data_scal* eat_str();

	bool scan_str(std::string& _str); // returns true on EOF
	bool unescape(std::string& _str); //
	void utf8_str(uint32_t _char, std::string& _str);

	inline bool is_space(src_i _c);
	inline bool eat_space(); // returns true on EOF

	const src_t m_src;
	src_i m_psrc;
};

#endif // #ifndef JSL_PARSER_H

/*

void test_parser()
{
	ESP_LOGI(LOGTAG, "Test PARSER");

	std::string str;
	if(!load_file("/config/test.json",str)) return;

	jsl_parser parser(str);
	jsl_data_dict* data = parser.parse();
	if(data != NULL)
	{
		ESP_LOGI(LOGTAG, "Data file parsed");
		std::cout << data->encode(true) << "\n\n";
		data->fire();
	}
	else ESP_LOGE(LOGTAG, "Failed to parse file");
}

*/
