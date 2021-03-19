open Kore;
open Util;

module Generator = Generate.Generator;

let __program = [
  of_decl(("ABC" |> of_public |> as_lexeme, 123 |> int_prim |> of_const)),
];

let suite =
  "Generate.Generator"
  >::: [
    "generate() - JavaScript"
    >: (
      () => {
        let buffer = Buffer.create(100);
        let print = Buffer.add_string(buffer);

        Generator.generate(
          JavaScript(ES6),
          {print, resolve: _ => ""},
          __program,
        );

        Assert.string(
          "import $knot from \"@knot/runtime\";
var ABC = 123;
export { ABC };
",
          buffer |> Buffer.contents,
        );
      }
    ),
  ];
