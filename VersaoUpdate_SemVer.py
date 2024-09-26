import configparser
import json
import os

# Carregar a versão do platformio.ini
config = configparser.ConfigParser()
config.read('platformio.ini')

# Buscar a versão a partir da macro definida
version = config['env:esp32doit-devkit-v1']['build_flags']
version_macro = [flag for flag in version.split('\n') if 'VERSION_MACRO' in flag]

if version_macro:
    version = version_macro[0].split('=')[1].strip().strip('"')

# Atualizar library.json
if os.path.exists('library.json'):
    with open('library.json', 'r+') as f:
        data = json.load(f)
        data['version'] = version
        f.seek(0)
        json.dump(data, f, indent=4)
        f.truncate()

# Atualizar library.properties
if os.path.exists('library.properties'):
    with open('library.properties', 'r+') as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            if line.startswith('version='):
                lines[i] = f'version={version}\n'
        f.seek(0)
        f.writelines(lines)
        f.truncate()
