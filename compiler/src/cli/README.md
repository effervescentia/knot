# knotc

![architecture](https://mermaid.ink/img/eyJjb2RlIjoiZmxvd2NoYXJ0IExSXG4gIGluKCggKSlcbiAgb3V0KCggKSlcblxuICBzdWJncmFwaCBrbm90LmV4ZWNcbiAgICBCdWlsZFxuICAgIEJ1bmRsZVxuICAgIERldmVsb3BcbiAgICBGb3JtYXRcbiAgICBMaW50XG4gICAgTFNQXG4gICAgV2F0Y2hcbiAgZW5kXG5cbiAgc3ViZ3JhcGgga25vdGNcbiAgICBNYWluXG4gIGVuZFxuXG4gIGluIC0tPiB8Y29tbWFuZHwga25vdC5leGVjXG5cbiAgQnVpbGQgLS0-IE1haW5cbiAgQnVuZGxlIC0tPiBNYWluXG4gIERldmVsb3AgLS0-IE1haW5cbiAgRm9ybWF0IC0tPiBNYWluXG4gIExpbnQgLS0-IE1haW5cbiAgTFNQIC0tPiBNYWluXG4gIFdhdGNoIC0tPiBNYWluXG5cbiAgTWFpbiAtLT4gb3V0XG4iLCJtZXJtYWlkIjp7InRoZW1lIjoiZGVmYXVsdCJ9LCJ1cGRhdGVFZGl0b3IiOmZhbHNlLCJhdXRvU3luYyI6dHJ1ZSwidXBkYXRlRGlhZ3JhbSI6ZmFsc2V9)

## Global Arguments

* `-c, --config [path]`: path to the configuration file (`.knot.yml`) used to define the target project. if any arguments are passed via command-line they override the values read from the config file.

## Command Arguments

* `-r, --root-dir [path]`: path to the root of the project. if a config file is found the root directory is treated as relative to the directory containing the config file but can also be absolue. all other project paths can be described relative to the root directory.

* `-s, --source-dir [path]`: path to the source code of the project. this path can be absolute or relative to the root directory.

* `-o, --out-dir [path]`: path to the output directory of the project. this path can be absolute or relative to the root directory.

* `-e, --entry [path]`: path to the entry file for the project. this path can be absolute or relative to the source directory.
