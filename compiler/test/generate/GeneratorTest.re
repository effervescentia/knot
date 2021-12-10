open Kore;
open Util.ResultUtil;

module Generator = Generate.Generator;

let __program = [
  (
    "ABC" |> of_public |> as_raw_node |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )
  |> of_decl
  |> as_raw_node,
];

let suite =
  "Generate.Generator"
  >::: [
    "generate() - JavaScript"
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
