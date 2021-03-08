open Test.Infix;

let () =
  "Unit Tests"
  >:::: TestCompile.TestSuite.tests
  @ TestFile.TestSuite.unit_tests
  @ TestGrammar.TestSuite.tests
  @ TestLibrary.TestSuite.tests
  @ TestParse.TestSuite.tests
  @ TestResolve.TestSuite.tests;
