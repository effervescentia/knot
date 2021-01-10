open Test.Infix;

let () =
  "Knot"
  >:::: TestLibrary.TestSuite.tests
  @ TestFile.TestSuite.tests
  @ TestGrammar.TestSuite.tests
  @ TestResolve.TestSuite.tests
  @ TestParse.TestSuite.tests
  @ TestCompile.TestSuite.tests;
