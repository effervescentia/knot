open Kore;
open Util;
open Reference;

module Import = Grammar.Import;

module Assert =
  Assert.Make({
    type t = AST.module_statement_t;

    let parser = ((_, ctx, _)) => Parser.parse(Import.parser(ctx));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              AST.Debug.print_mod_stmt
              % Pretty.to_string
              % Format.pp_print_string(pp),
            (==),
          ),
          "program matches",
        )
      );
  });

let __scope_tree = BinaryTree.create((Cursor.zero |> Cursor.expand, None));

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
             ~ns_context=
               NamespaceContext.create(
                 ~modules=
                   AST.[
                     (
                       "bar" |> of_internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [
                             (Export.Main, Type.Valid(`Nil)),
                             (
                               Export.Named("bar" |> of_public),
                               Type.Valid(`Boolean),
                             ),
                             (
                               Export.Named("foo" |> of_public),
                               Type.Valid(`String),
                             ),
                           ]
                           |> List.to_seq
                           |> Hashtbl.of_seq,
                         scopes: __scope_tree,
                         raw: "foo",
                       },
                     ),
                   ]
                   |> List.to_seq
                   |> Hashtbl.of_seq,
                 Internal("mock"),
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
             ~ns_context=
               NamespaceContext.create(
                 ~modules=
                   AST.[
                     (
                       "bar" |> of_internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [(Export.Main, Type.Valid(`Nil))]
                           |> List.to_seq
                           |> Hashtbl.of_seq,
                         scopes: __scope_tree,
                         raw: "foo",
                       },
                     ),
                   ]
                   |> List.to_seq
                   |> Hashtbl.of_seq,
                 Internal("mock"),
               ),
           )
    ),
  ];
