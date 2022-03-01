open Kore;

let tests = [
  Semantic_ArgumentTest.suite,
  Semantic_BinaryOpTest.suite,
  Semantic_ExpressionTest.suite,
  Semantic_JSXTest.suite,
  Semantic_StatementTest.suite,
  Semantic_UnaryOpTest.suite,
  Typing_BinaryOpTest.suite,
  Typing_JSXTest.suite,
  Typing_TypeExpressionTest.suite,
  Typing_UnaryOpTest.suite,
];
