open Kore;

let () = {
  "Grammar"
  >:::: [
    ProgramTest.suite,
    Primitive_NilTest.suite,
    Primitive_BooleanTest.suite,
    Primitive_NumberTest.suite,
    Primitive_StringTest.suite,
  ];
};
