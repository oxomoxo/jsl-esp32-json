## A lean json parser and data tree. Handcrafted for esp32.

### Use

```cpp
	std::string test;

	if(!load_file("/test.json",test)) return;

	jsl_parser parser(test);
	jsl_data* data = parser.parse();
	if(data != NULL)
	{
		ESP_LOGI(PARSER_TEST_LOGTAG, "Data file parsed");
		std::cout << data->encode(true) << "\n\n";
		data->fire();
	}
	else ESP_LOGE(PARSER_TEST_LOGTAG, "Failed to parse file");
```

The above code snippet
- Loads a json file into a string
- creates an instance of the parser
- parses the file and outputs a data object
- prints out a json string from the tree

### Install

```bash
git clone https://github.com/oxomoxo/jsl-esp32-json.git json
```
In component.mk add the folder to the COMPONENT_ADD_INCLUDEDIRS and COMPONENT_SRCDIRS

```mk
COMPONENT_ADD_INCLUDEDIRS := . \
	json \

COMPONENT_SRCDIRS := . \
	json \
```

Build :

```bash
make -j4 flash monitor
```

And, Voila !
Neat isnt it ?
