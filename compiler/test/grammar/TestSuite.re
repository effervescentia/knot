open Kore;

let tests = [
  NumberTest.suite,
  Primitive_NilTest.suite,
  Primitive_BooleanTest.suite,
  Primitive_StringTest.suite,
  JSXTest.suite,
  ExpressionTest.suite,
  Declaration_ConstantTest.suite,
  ImportTest.suite,
  ProgramTest.suite,
];
