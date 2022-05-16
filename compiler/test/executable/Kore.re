include Knot.Kore;
include Test.Infix;

module Async = Test.Async;

let fixture_dir = "./test/executable/.fixtures";

let _fixture = Filename.concat(fixture_dir);

let simple_fixture_dir = _fixture("simple");
let complex_fixture_dir = _fixture("complex");
let messy_fixture_dir = _fixture("messy");
let messy_cyclic_fixture_dir = _fixture("messy_cyclic");
let messy_missing_import_fixture_dir = _fixture("messy_missing_import");
let no_target_fixture_dir = _fixture("no_target");
let js_target_fixture_dir = _fixture("js_target");
let path_alternative_fixture_dir = _fixture("path_alternative");
let path_override_fixture_dir = _fixture("path_override");
let bool_flags_fixture_dir = _fixture("bool_flags");
let global_config_fixture_dir = _fixture("global_config");
let nested_fixture_dir = _fixture("nested");

let full_config_file = _fixture("full_config.yml");
let unexpected_prop_config_file = _fixture("unexpected_prop_config.yml");
let invalid_format_config_file = _fixture("invalid_format_config.yml");
let cannot_parse_config_file = _fixture("cannot_parse_config.yml");
