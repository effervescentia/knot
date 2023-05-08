include Interface;

type import_spec_t = Formatter.import_spec_t;

let parse = Parser.parse;

let format = Formatter.format;
let format_namespace = Formatter.format_namespace;
let format_named_import_list = Formatter.format_named_import_list;

let to_xml = Debug.to_xml;
let stdlib_import_to_xml = Debug.stdlib_import_to_xml;
