open Kore;
open Util.CommonUtil;
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
              (
                "bar" |> of_internal,
                [
                  "foo"
                  |> of_public
                  |> as_raw_node
                  |> of_main_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
            ),
          ),
          (
            "import {} from \"@/bar\"",
            AST.(("bar" |> of_internal, []) |> of_import |> as_raw_node),
          ),
          (
            "import { foo } from \"@/bar\"",
            AST.(
              (
                "bar" |> of_internal,
                [
                  ("foo" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
            ),
          ),
          (
            "import { foo as bar } from \"@/bar\"",
            AST.(
              (
                "bar" |> of_internal,
                [
                  (
                    "foo" |> of_public |> as_raw_node,
                    Some("bar" |> of_public |> as_raw_node),
                  )
                  |> of_named_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
            ),
          ),
          (
            "import fizz, { foo, bar as Bar } from \"@/bar\"",
            AST.(
              (
                "bar" |> of_internal,
                [
                  "fizz"
                  |> of_public
                  |> as_raw_node
                  |> of_main_import
                  |> as_raw_node,
                  ("foo" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                  (
                    "bar" |> of_public |> as_raw_node,
                    Some("Bar" |> of_public |> as_raw_node),
                  )
                  |> of_named_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
            ),
          ),
          (
            "import { foo, bar, } from \"@/bar\"",
            AST.(
              (
                "bar" |> of_internal,
                [
                  ("foo" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                  ("bar" |> of_public |> as_raw_node, None)
                  |> of_named_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
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
                         exports:
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
              (
                "bar" |> of_internal,
                [
                  "foo"
                  |> of_public
                  |> as_raw_node
                  |> of_main_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
            ),
          ),
          (
            "  import  foo  from   \"@/bar\"  ;  ",
            AST.(
              (
                "bar" |> of_internal,
                [
                  "foo"
                  |> of_public
                  |> as_raw_node
                  |> of_main_import
                  |> as_raw_node,
                ],
              )
              |> of_import
              |> as_raw_node
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
                         exports:
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
