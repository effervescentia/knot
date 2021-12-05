open Test.Infix;

let () =
  "Unit Tests"
  >:::: TestCompile.TestSuite.tests
  @ TestLanguageServer.TestSuite.tests
  @ TestExecutable.TestSuite.unit_tests
  @ TestFile.TestSuite.unit_tests
  @ TestGenerate.TestSuite.tests
  @ TestGrammar.TestSuite.tests
  @ TestLibrary.TestSuite.tests
  @ TestParse.TestSuite.tests
  @ TestResolve.TestSuite.tests;
