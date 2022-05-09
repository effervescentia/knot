open Kore;

let unit_tests = [
  BuildTest.suite,
  ArgumentsTest.suite,
  ReporterTest.suite,
  TaskTest.suite,
  UsageTest.suite,
];

let integration_tests = [FormatTest.suite];
