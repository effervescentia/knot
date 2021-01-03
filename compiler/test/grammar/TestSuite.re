open Kore;

let () = {
  "Grammar"
  >:::: [
    Primitive_NilTest.suite,
    Primitive_BooleanTest.suite,
    Primitive_NumberTest.suite,
    Primitive_StringTest.suite,
    ExpressionTest.suite,
    Declaration_ConstantTest.suite,
    ImportTest.suite,
    ProgramTest.suite,
  ];
};
