open Kore;

module Generator = Generate.Generator;
module U = Util.ResultUtil;

let __program = [
  (
    "ABC" |> A.of_public |> U.as_raw_node |> A.of_named_export,
    123 |> U.int_prim |> A.of_const |> U.as_int,
  )
  |> A.of_decl
  |> U.as_raw_node,
];

let suite =
  "Generate.Generator"
  >::: [
    "generate() - javascript with es6 modules"
    >: (
      () =>
        Assert.string(
          "import $knot from \"@knot/runtime\";
var ABC = 123;
export { ABC };
",
          __program
          |> ~@(
               (ppf, x) =>
                 Generator.generate(JavaScript(ES6), _ => "", x, ppf)
             ),
        )
    ),
  ];
