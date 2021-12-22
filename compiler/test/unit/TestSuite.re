open Test.Infix;

let () =
  "Unit Tests"
  >:::: TestAnalyze.TestSuite.tests
  @ TestPretty.TestSuite.tests
  @ TestLibrary.TestSuite.tests
  @ TestFile.TestSuite.unit_tests
  @ TestExecutable.TestSuite.unit_tests
  @ TestGrammar.TestSuite.tests
  @ TestGenerate.TestSuite.tests;
/*
 let () =
   "Unit Tests"
   >:::: TestAnalyze.TestSuite.tests
   @ TestCompile.TestSuite.tests
   @ TestLanguageServer.TestSuite.tests
   @ TestExecutable.TestSuite.unit_tests
   @ TestFile.TestSuite.unit_tests
   @ TestGenerate.TestSuite.tests
   @ TestGrammar.TestSuite.tests
   @ TestLibrary.TestSuite.tests
   @ TestParse.TestSuite.tests
   @ TestPretty.TestSuite.tests
   @ TestResolve.TestSuite.tests; */
