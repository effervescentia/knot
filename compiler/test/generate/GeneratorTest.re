open Kore;

module A = AST.Result;
module Generator = Generate.Generator;
module U = Util.ResultUtil;

let __program = [
  (
    AST.ModuleStatement.ExportKind.Named,
    "ABC" |> U.as_untyped,
    123 |> U.int_prim |> A.of_const |> U.as_int,
  )
  |> A.of_export
  |> U.as_untyped,
];

let suite =
  "Generate.Generator"
  >::: [
    "pp() - javascript with es6 modules"
    >: (
      () =>
        Assert.string(
          "import $knot from \"@knot/runtime\";
var ABC = 123;
export { ABC };
",
          __program |> ~@Generator.pp(JavaScript(ES6), _ => ""),
        )
    ),
  ];
