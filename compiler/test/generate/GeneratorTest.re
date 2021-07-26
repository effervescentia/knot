open Kore;
open Util.ResultUtil;

module Generator = Generate.Generator;

let __program = [
  of_decl((
    "ABC" |> of_public |> as_lexeme |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )),
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
          Generator.generate(JavaScript(ES6), _ => "", __program)
          |> Pretty.to_string,
        )
    ),
  ];
