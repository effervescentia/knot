open Kore;

let tests = [
  Declaration_ConstantTest.suite,
  Declaration_FunctionTest.suite,
  Declaration_ViewTest.suite,
  DeclarationTest.suite,
  ExpressionTest.suite,
  Formatter_DeclarationTest.suite,
  Formatter_ExpressionTest.suite,
  Formatter_ImportTest.suite,
  Formatter_JSXTest.suite,
  Formatter_ModuleTest.suite,
  FormatterTest.suite,
  IdentifierTest.suite,
  ImportTest.suite,
  JSXTest.suite,
  NumberTest.suite,
  Primitive_BooleanTest.suite,
  Primitive_NilTest.suite,
  Primitive_StringTest.suite,
  ProgramTest.suite,
  StatementTest.suite,
  TypingTest.suite,
];
