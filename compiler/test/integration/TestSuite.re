open Test.Infix;

let () =
  "Integration Tests"
  >~:::: TestFile.TestSuite.integration_tests
  @ TestExecutable.TestSuite.integration_tests;
