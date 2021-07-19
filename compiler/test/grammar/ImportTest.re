open Kore;
open AST.Raw.Util;
open Reference;
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

let __scope_tree = (Cursor.zero |> Cursor.expand, None) |> BinaryTree.create;

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
            (to_internal("bar"), [to_public_main_import("foo")])
            |> to_import,
          ),
          (
            "import {} from \"@/bar\"",
            (to_internal("bar"), []) |> to_import,
          ),
          (
            "import { foo } from \"@/bar\"",
            (
              to_internal("bar"),
              [(raw_public("foo"), None) |> to_named_import],
            )
            |> to_import,
          ),
          (
            "import { foo as bar } from \"@/bar\"",
            (
              to_internal("bar"),
              [
                (raw_public("foo"), Some(raw_public("bar")))
                |> to_named_import,
              ],
            )
            |> to_import,
          ),
          (
            "import fizz, { foo, bar as Bar } from \"@/bar\"",
            (
              "bar" |> to_internal,
              [
                to_public_main_import("fizz"),
                (raw_public("foo"), None) |> to_named_import,
                (raw_public("bar"), Some(raw_public("Bar")))
                |> to_named_import,
              ],
            )
            |> to_import,
          ),
          (
            "import { foo, bar, } from \"@/bar\"",
            (
              "bar" |> to_internal,
              [
                (raw_public("foo"), None) |> to_named_import,
                (raw_public("bar"), None) |> to_named_import,
              ],
            )
            |> to_import,
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               Scope.create(
                 ~modules=
                   [
                     (
                       "bar" |> to_internal,
                       ModuleTable.{
                         ast: [],
                         types:
                           [
                             (Export.Main, Type.K_Strong(K_Nil)),
                             (
                               Export.Named("bar" |> to_public),
                               Type.K_Strong(K_Boolean),
                             ),
                             (
                               Export.Named("foo" |> to_public),
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
            (to_internal("bar"), [to_public_main_import("foo")])
            |> to_import,
          ),
          (
            "  import  foo  from   \"@/bar\"  ;  ",
            (to_internal("bar"), [to_public_main_import("foo")])
            |> to_import,
          ),
        ]
        |> Assert.parse_many(
             ~scope=
               Scope.create(
                 ~modules=
                   [
                     (
                       "bar" |> to_internal,
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
