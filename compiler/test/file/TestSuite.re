open Kore;

let unit_tests = [
  CacheTest.suite,
  CharStreamTest.suite,
  IOTest.suite,
  WatcherTest_Unit.suite,
];

let integration_tests = [WatcherTest_IT.suite];
