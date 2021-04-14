open Kore;
open Util;
open Reference;

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
                ["foo" |> of_public |> as_lexeme |> of_main_import],
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
                [("foo" |> of_public |> as_lexeme, None) |> of_named_import],
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
                  |> of_named_import,
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
                  "fizz" |> of_public |> as_lexeme |> of_main_import,
                  ("foo" |> of_public |> as_lexeme, None) |> of_named_import,
                  (
                    "bar" |> of_public |> as_lexeme,
                    Some("Bar" |> of_public |> as_lexeme),
                  )
                  |> of_named_import,
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
                  ("foo" |> of_public |> as_lexeme, None) |> of_named_import,
                  ("bar" |> of_public |> as_lexeme, None) |> of_named_import,
                ],
              ))
            ),
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               Scope.create(
                 ~modules=
                   AST.[
                     (
                       "bar" |> of_internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [
                             (Export.Main, Type.K_Strong(K_Nil)),
                             (
                               Export.Named("bar" |> of_public),
                               Type.K_Strong(K_Boolean),
                             ),
                             (
                               Export.Named("foo" |> of_public),
                               Type.K_Strong(K_String),
                             ),
                           ]
                           |> List.to_seq
                           |> Hashtbl.of_seq,
                       },
                     ),
                   ]
                   |> List.to_seq
                   |> Hashtbl.of_seq,
                 (),
               ),
           )
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
                ["foo" |> of_public |> as_lexeme |> of_main_import],
              ))
            ),
          ),
          (
            "  import  foo  from   \"@/bar\"  ;  ",
            AST.(
              of_import((
                "bar" |> of_internal,
                ["foo" |> of_public |> as_lexeme |> of_main_import],
              ))
            ),
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               Scope.create(
                 ~modules=
                   AST.[
                     (
                       "bar" |> of_internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [(Export.Main, Type.K_Strong(K_Nil))]
                           |> List.to_seq
                           |> Hashtbl.of_seq,
                       },
                     ),
                   ]
                   |> List.to_seq
                   |> Hashtbl.of_seq,
                 (),
               ),
           )
    ),
  ];
