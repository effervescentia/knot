open Test.Infix;

let () = "Unit Tests" >:::: TestPretty.TestSuite.tests;
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
