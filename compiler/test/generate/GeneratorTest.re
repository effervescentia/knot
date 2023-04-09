open Kore;

module Generator = Generate.Generator;
module U = Util.ResultUtil;

let __program = [
  (
    KModuleStatement.Interface.ExportKind.Named,
    "ABC" |> U.as_untyped,
    123 |> U.int_prim |> KDeclaration.Interface.of_constant |> U.as_int,
  )
  |> KModuleStatement.Interface.of_export
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
