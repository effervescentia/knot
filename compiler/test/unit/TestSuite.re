open Test.Infix;

let () =
  "Unit Tests"
  >:::: TestLibrary.TestSuite.tests
  @ TestFile.TestSuite.unit_tests
  @ TestGrammar.TestSuite.tests
  @ TestResolve.TestSuite.tests
  @ TestParse.TestSuite.tests
  @ TestCompile.TestSuite.tests;
