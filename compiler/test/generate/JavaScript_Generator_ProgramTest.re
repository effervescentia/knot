open Kore;
open Generate.JavaScript_AST;

module A = AST.Result;
module Generator = Generate.JavaScript_Generator;
module Formatter = Generate.JavaScript_Formatter;
module U = Util.ResultUtil;

let __resolved = "../foo/bar";
let __program =
  [
    ("foo/bar" |> A.of_internal, "Foo" |> U.as_untyped |> Option.some, [])
    |> A.of_import,
    [
      (U.as_untyped("Fizz"), "Buzz" |> U.as_untyped |> Option.some)
      |> U.as_untyped,
    ]
    |> A.of_standard_import,
    (
      AST.Module.Named,
      "ABC" |> U.as_untyped,
      123 |> U.int_prim |> A.of_const |> U.as_int,
    )
    |> A.of_export,
  ]
  |> List.map(U.as_untyped);

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
          (U.as_untyped("foo"), 123 |> U.int_prim |> A.of_const |> U.as_int),
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
