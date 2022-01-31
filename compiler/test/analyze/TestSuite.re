open Kore;

let tests = [
  Analyzer_AddOpTest.suite,
  Analyzer_ExpressionTest.suite,
  Analyzer_IdentifierTest.suite,
  Analyzer_JSXTest.suite,
  Analyzer_NegativeOpTest.suite,
  Analyzer_NotOpTest.suite,
  Analyzer_PositiveOpTest.suite,
  Analyzer_PrimitiveTest.suite,
  Analyzer_StatementTest.suite,
];
