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
              AST.Raw.Debug.print_mod_stmt
              % Cow.Xml.list
              % Cow.Xml.to_string
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
              (
                "bar" |> RawUtil.internal,
                ["foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import
            ),
          ),
          (
            "import {} from \"@/bar\"",
            AST.(("bar" |> RawUtil.internal, []) |> RawUtil.import),
          ),
          (
            "import { foo } from \"@/bar\"",
            AST.(
              (
                "bar" |> RawUtil.internal,
                [
                  ("foo" |> RawUtil.public |> as_lexeme, None)
                  |> RawUtil.named_import,
                ],
              )
              |> RawUtil.import
            ),
          ),
          (
            "import { foo as bar } from \"@/bar\"",
            AST.(
              (
                "bar" |> RawUtil.internal,
                [
                  (
                    "foo" |> RawUtil.public |> as_lexeme,
                    Some("bar" |> RawUtil.public |> as_lexeme),
                  )
                  |> RawUtil.named_import,
                ],
              )
              |> RawUtil.import
            ),
          ),
          (
            "import fizz, { foo, bar as Bar } from \"@/bar\"",
            AST.(
              (
                "bar" |> RawUtil.internal,
                [
                  "fizz" |> RawUtil.public |> as_lexeme |> RawUtil.main_import,
                  ("foo" |> RawUtil.public |> as_lexeme, None)
                  |> RawUtil.named_import,
                  (
                    "bar" |> RawUtil.public |> as_lexeme,
                    Some("Bar" |> RawUtil.public |> as_lexeme),
                  )
                  |> RawUtil.named_import,
                ],
              )
              |> RawUtil.import
            ),
          ),
          (
            "import { foo, bar, } from \"@/bar\"",
            AST.(
              (
                "bar" |> RawUtil.internal,
                [
                  ("foo" |> RawUtil.public |> as_lexeme, None)
                  |> RawUtil.named_import,
                  ("bar" |> RawUtil.public |> as_lexeme, None)
                  |> RawUtil.named_import,
                ],
              )
              |> RawUtil.import
            ),
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               Scope.create(
                 ~modules=
                   AST.[
                     (
                       "bar" |> RawUtil.internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [
                             (Export.Main, Type.K_Strong(K_Nil)),
                             (
                               Export.Named("bar" |> RawUtil.public),
                               Type.K_Strong(K_Boolean),
                             ),
                             (
                               Export.Named("foo" |> RawUtil.public),
                               Type.K_Strong(K_String),
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
              (
                "bar" |> RawUtil.internal,
                ["foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import
            ),
          ),
          (
            "  import  foo  from   \"@/bar\"  ;  ",
            AST.(
              (
                "bar" |> RawUtil.internal,
                ["foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
              )
              |> RawUtil.import
            ),
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               Scope.create(
                 ~modules=
                   AST.[
                     (
                       "bar" |> RawUtil.internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [(Export.Main, Type.K_Strong(K_Nil))]
                           |> List.to_seq
                           |> Hashtbl.of_seq,
                         scopes: __scope_tree,
                         raw: "foo",
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
