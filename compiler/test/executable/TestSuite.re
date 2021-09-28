open Kore;

let unit_tests = [
  ArgsTest.suite,
  BuildTest.suite,
  ConfigOptTest.suite,
  OptTest.suite,
  RunCmdTest.suite,
];

let integration_tests = [FormatTest.suite];
