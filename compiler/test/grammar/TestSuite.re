open Kore;

let tests = [
  Declaration_ConstantTest.suite,
  Declaration_FunctionTest.suite,
  DeclarationTest.suite,
  ExpressionTest.suite,
  FormatterTest.suite,
  ImportTest.suite,
  JSXTest.suite,
  NumberTest.suite,
  Primitive_BooleanTest.suite,
  Primitive_NilTest.suite,
  Primitive_StringTest.suite,
  ProgramTest.suite,
];
