open Kore;

let tests = [
  Declaration_ConstantTest.suite,
  Declaration_FunctionTest.suite,
  DeclarationTest.suite,
  ExpressionTest.suite,
  Formatter_DeclarationTest.suite,
  Formatter_ExpressionTest.suite,
  Formatter_ImportTest.suite,
  Formatter_JSXTest.suite,
  Formatter_ModuleTest.suite,
  FormatterTest.suite,
  ImportTest.suite,
  JSXTest.suite,
  NumberTest.suite,
  Primitive_BooleanTest.suite,
  Primitive_BooleanTestV2.suite,
  Primitive_NilTest.suite,
  Primitive_NilTestV2.suite,
  Primitive_StringTest.suite,
  Primitive_StringTestV2.suite,
  ProgramTest.suite,
];
