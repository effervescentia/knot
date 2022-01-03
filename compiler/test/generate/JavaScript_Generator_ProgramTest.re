open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;

let __resolved = "../foo/bar";
let __program =
  [
    (
      "foo/bar" |> of_internal,
      ["Foo" |> of_public |> as_raw_node |> of_main_import |> as_raw_node],
    )
    |> of_import,
    (
      "ABC" |> of_public |> as_raw_node |> of_named_export,
      123 |> int_prim |> of_const |> as_int,
    )
    |> of_decl,
  ]
  |> List.map(as_raw_node);

let _assert_declaration = (expected, actual) =>
  Alcotest.(
    check(
      list(Assert.Compare.statement(Target.Common)),
      "javascript declaration matches",
      expected,
      actual |> Tuple.join2(Generator.gen_declaration),
    )
  );
let _assert_program = (expected, actual) =>
  Alcotest.(
    check(
      list(Assert.Compare.statement(Target.Common)),
      "javascript program matches",
      expected,
      actual |> Tuple.join2(Generator.generate),
    )
  );

let suite =
  "Generate.JavaScript_Generator | Program"
  >::: [
    "declaration() - public"
    >: (
      () =>
        _assert_declaration(
          [Variable("foo", Number("123")), Export("foo", None)],
          (
            "foo" |> of_public |> as_raw_node,
            123 |> int_prim |> of_const |> as_int,
          ),
        )
    ),
    "declaration() - private"
    >: (
      () =>
        _assert_declaration(
          [Variable("_foo", Number("123"))],
          (
            "foo" |> of_private |> as_raw_node,
            123 |> int_prim |> of_const |> as_int,
          ),
        )
    ),
    "generate() - empty module"
    >: (() => _assert_program([EmptyExport], (_ => "", []))),
    "generate() - with imports and exports"
    >: (
      () =>
        _assert_program(
          [
            DefaultImport("@knot/runtime", "$knot"),
            Import("../foo/bar", [("main", Some("Foo"))]),
            Variable("ABC", Number("123")),
            Export("ABC", None),
          ],
          (
            path => {
              Assert.namespace("foo/bar" |> of_internal, path);
              __resolved;
            },
            __program,
          ),
        )
    ),
  ];
