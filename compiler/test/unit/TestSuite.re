open Test.Infix;

let () =
  "Unit Tests"
  >:::: TestLibrary.TestSuite.tests
  @ TestPretty.TestSuite.tests
  @ TestFile.TestSuite.unit_tests
  @ TestParse.TestSuite.tests
  @ TestResolve.TestSuite.tests
  @ TestGrammar.TestSuite.tests
  @ TestAnalyze.TestSuite.tests
  @ TestGenerate.TestSuite.tests
  @ TestCompile.TestSuite.tests
  @ TestExecutable.TestSuite.unit_tests
  @ TestLanguageServer.TestSuite.tests;
