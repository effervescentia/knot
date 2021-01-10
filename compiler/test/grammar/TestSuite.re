open Kore;

let tests = [
  Primitive_NilTest.suite,
  Primitive_BooleanTest.suite,
  Primitive_NumberTest.suite,
  Primitive_StringTest.suite,
  JSXTest.suite,
  ExpressionTest.suite,
  Declaration_ConstantTest.suite,
  ImportTest.suite,
  ProgramTest.suite,
];
