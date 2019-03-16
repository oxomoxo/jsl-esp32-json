/*
	test.cpp

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




#include "../jsl-parser.h"

#define PARSER_TEST_LOGTAG "PARSER-TEST :"
#include <esp_log.h>

bool load_file(const char* _fname, std::ostringstream& _dest)
{
	std::ifstream file(_fname, std::ios::binary);
	if(file.is_open())
	{
		ESP_LOGI(PARSER_TEST_LOGTAG, "File open [%s]",_fname);

		_dest << file.rdbuf();
		file.close();

		return true;
	}

	ESP_LOGE(PARSER_TEST_LOGTAG, "Failed to open [%s] for reading",_fname);

	return false;
}

bool load_file(const char* _fname, std::string& _dest)
{
	std::ostringstream buf;

	if(!load_file(_fname,buf)) return false;

	_dest = buf.str();
	return true;
}

void test_parser()
{
	ESP_LOGI(PARSER_TEST_LOGTAG, "Test PARSER");

	jsl_data_pool::init(100,20,20);

	std::string test;

	if(!load_file("/test.json",test)) return;

	jsl_parser parser(test);
	jsl_data_dict* data = parser.parse();
	if(data != nullptr)
	{
		ESP_LOGI(PARSER_TEST_LOGTAG, "Data file parsed");
		std::cout << data->encode(true) << "\n\n";
		data->fire();
	}
	else ESP_LOGE(PARSER_TEST_LOGTAG, "Failed to parse file");

	jsl_data_pool::init(0,0,0);
}
