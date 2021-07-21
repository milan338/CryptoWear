# Get a JSON object containing currency codes with their name and symbols

import json
from urllib.request import urlopen
from os.path import abspath, dirname, join

URL = 'https://gist.githubusercontent.com/Fluidbyte/2973986/raw/8bb35718d0c90fdacb388961c98b8d56abc392c9/Common-Currency.json'
FILE_NAME = 'currency_data.json'
FILE_PATH = 'res/contents/data'

# Get data
response = urlopen(URL)
data = json.loads(response.read())
base_path = abspath(dirname(__file__))
json_dir = abspath(join(base_path, '..', FILE_PATH))
json_path = abspath(join(json_dir, FILE_NAME))

# Irrelevant JSON keys
exclude = (
    'symbol_native',
    'decimal_digits',
    'rounding',
    'code',
    'name_plural')

# Filter JSON
in_json = {}
for currency in data.keys():
    in_json[currency] = {}
    for key, value in data[currency].items():
        if key not in exclude:
            in_json[currency][key] = value

# Sort JSON by currency name
out_json = {}
for key, value in sorted(in_json.items(), key=lambda i: i[1]['name']):
    out_json[key] = value

# Array of currency codes sorted by currency name
out_list = [key for key in out_json.keys()]
for i in out_list:
    print(f'"{i}",')

# Dump JSON to file
try:
    with open(json_path, 'w', encoding='utf8') as file:
        json.dump(out_json, file, indent=4,
                  sort_keys=False, ensure_ascii=False)
    print(f'Successfully written data to {json_path}')
except FileNotFoundError:
    print(f'Invalid path, {json_dir} does not exist')
except:
    print(f'Failed to write to write data to {json_path}')
