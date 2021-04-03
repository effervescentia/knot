open Kore;
open Util;

module Import = Grammar.Import;

module Assert =
  Assert.Make({
    type t = AST.module_statement_t;

    let parser = ctx => Parser.parse(Import.parser(ctx));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Debug.print_mod_stmt
              % Pretty.to_string
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Import"
  >::: [
    "no parse"
    >: (
      () =>
        ["gibberish", "import", "import foo", "import foo from"]
        |> Assert.no_parse
    ),
    "parse"
    >: (
      () =>
        [
          (
            "import foo from \"@/bar\"",
            AST.(
              of_import((
                "bar" |> of_internal,
                ["foo" |> of_public |> as_lexeme |> of_main],
              ))
            ),
          ),
          (
            "import {} from \"@/bar\"",
            AST.(of_import(("bar" |> of_internal, []))),
          ),
          (
            "import { foo } from \"@/bar\"",
            AST.(
              of_import((
                "bar" |> of_internal,
                [("foo" |> of_public |> as_lexeme, None) |> of_named],
              ))
            ),
          ),
          (
            "import { foo as bar } from \"@/bar\"",
            AST.(
              of_import((
                "bar" |> of_internal,
                [
                  (
                    "foo" |> of_public |> as_lexeme,
                    Some("bar" |> of_public |> as_lexeme),
                  )
                  |> of_named,
                ],
              ))
            ),
          ),
          (
            "import fizz, { foo, bar as Bar } from \"@/bar\"",
            AST.(
              of_import((
                "bar" |> of_internal,
                [
                  "fizz" |> of_public |> as_lexeme |> of_main,
                  ("foo" |> of_public |> as_lexeme, None) |> of_named,
                  (
                    "bar" |> of_public |> as_lexeme,
                    Some("Bar" |> of_public |> as_lexeme),
                  )
                  |> of_named,
                ],
              ))
            ),
          ),
          (
            "import { foo, bar, } from \"@/bar\"",
            AST.(
              of_import((
                "bar" |> of_internal,
                [
                  ("foo" |> of_public |> as_lexeme, None) |> of_named,
                  ("bar" |> of_public |> as_lexeme, None) |> of_named,
                ],
              ))
            ),
          ),
        ]
        |> Assert.parse_many
    ),
    "parse terminated"
    >: (
      () =>
        [
          (
            "import foo from \"@/bar\";",
            AST.(
              of_import((
                "bar" |> of_internal,
                ["foo" |> of_public |> as_lexeme |> of_main],
              ))
            ),
          ),
          (
            "  import  foo  from   \"@/bar\"  ;  ",
            AST.(
              of_import((
                "bar" |> of_internal,
                ["foo" |> of_public |> as_lexeme |> of_main],
              ))
            ),
          ),
        ]
        |> Assert.parse_many
    ),
  ];
