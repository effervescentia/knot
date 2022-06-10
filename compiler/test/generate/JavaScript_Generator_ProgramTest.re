open Kore;
open Generate.JavaScript_AST;

module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;
module U = Util.ResultUtil;

let __resolved = "../foo/bar";
let __program =
  [
    (
      "foo/bar" |> A.of_internal,
      [
        "Foo"
        |> A.of_public
        |> U.as_raw_node
        |> A.of_main_import
        |> U.as_raw_node,
      ],
    )
    |> A.of_import,
    [
      (
        "Fizz" |> A.of_public |> U.as_raw_node,
        "Buzz" |> A.of_public |> U.as_raw_node |> Option.some,
      )
      |> U.as_raw_node,
    ]
    |> A.of_standard_import,
    (
      "ABC" |> A.of_public |> U.as_raw_node |> A.of_named_export,
      123 |> U.int_prim |> A.of_const |> U.as_int,
    )
    |> A.of_decl,
  ]
  |> List.map(U.as_raw_node);

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
            "foo" |> A.of_public |> U.as_raw_node,
            123 |> U.int_prim |> A.of_const |> U.as_int,
          ),
        )
    ),
    "declaration() - private"
    >: (
      () =>
        _assert_declaration(
          [Variable("_foo", Number("123"))],
          (
            "foo" |> A.of_private |> U.as_raw_node,
            123 |> U.int_prim |> A.of_const |> U.as_int,
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
            Variable(
              "Buzz",
              DotAccess(DotAccess(Identifier("$knot"), "stdlib"), "Fizz"),
            ),
            Variable("ABC", Number("123")),
            Export("ABC", None),
          ],
          (
            path => {
              Assert.namespace("foo/bar" |> A.of_internal, path);
              __resolved;
            },
            __program,
          ),
        )
    ),
  ];
