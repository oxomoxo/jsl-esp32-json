## A lean C++ json parser and data tree. Handcrafted for esp32

### What is it

For some project I'm working on I needed a json solution, but the available components were not fit (cjson is C and has a terrible interface, jsmn is not even near a json parser, and some other projects like Niels Lohmann jsoncpp is great but, gasp! 20K loc !!) so I decided to code my own.

It's small (< 2K loc) it's neat and compact.

### Use

```cpp
#include "json/jsl-parser.h"

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
```

The above code snippet
- inits the pool to some value
- Loads a json file into a string
- creates an instance of the parser
- parses the file and outputs a data object
- prints out a json string from the tree
- releases the pool

the test/test.json file contains the following data :

```json
{
	"object": {
		"string": "Some string with a lot of ecaped characters and some UTF-8 characters : \\ \" \/ \f \b \n \r \t \u03A9-Ω-\u03C9-ω",
		"number_uint": 123456,
		"number_sint": -123456,
		"number_real": 1.23456,
		"number_zreal": 0.123456,
		"number_sreal": -1.23456,
		"number_szreal": -0.123456,
		"number_expo": 1.2345e+6,
		"number_sexpo": -1.2345e+6,
		"number_nexpo": 1.2345e-6,
		"number_snexpo": -1.2345e-6,
		"true": true,
		"false": false,
		"null": null
	},
	"array": [
		"Some string with a lot of ecaped characters and some UTF-8 characters : \\ \" \/ \f \b \n \r \t \u03A9-Ω-\u03C9-ω",
		123456,
		-123456,
		1.23456,
		0.123456,
		-1.23456,
		-0.123456,
		1.2345e+6,
		-1.2345e+6,
		1.2345e-6,
		-1.2345e-6,
		true,
		false,
		null
	]
}
```

It's aimed at testing all Number permutations and UTF-8 input values (in addition to Object, Array, and simple constants).


### Install

```bash
git clone https://github.com/oxomoxo/jsl-esp32-json.git json
```
Or
```bash
git submodule add https://github.com/oxomoxo/jsl-esp32-json.git json
```
In component.mk add the folder to the `COMPONENT_ADD_INCLUDEDIRS` and `COMPONENT_SRCDIRS`

```mk
COMPONENT_ADD_INCLUDEDIRS := . \
	...
	json \

COMPONENT_SRCDIRS := . \
	...
	json \
```

Build :

```bash
make -j4 flash monitor
```

And, Voila !

Neat isn't it ?
