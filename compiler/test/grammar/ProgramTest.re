open Kore;
open AST;
open Util;
open ResultUtil;
open Reference;

module Program = Grammar.Program;

module Target = {
  type t = program_t;

  let parser = ((ctx, _, _)) => ctx |> Program.main |> Parser.parse;

  let test =
    Alcotest.(
      check(
        list(
          testable(
            pp =>
              Debug.print_mod_stmt
              % Pretty.to_string
              % Format.pp_print_string(pp),
            (==),
          ),
        ),
        "program matches",
      )
    );
};

module AssertImports =
  Assert.Make({
    include Target;

    let parser = ((ctx, _, _)) => ctx |> Program.imports |> Parser.parse;
  });
module Assert = Assert.Make(Target);

let __main_import = "import foo from \"@/bar\"";
let __const_decl = "const foo = nil";
let __scope_tree = BinaryTree.create((Cursor.zero |> Cursor.expand, None));

let __main_import_ast =
  ("bar" |> of_internal, ["foo" |> of_public |> as_lexeme |> of_main_import])
  |> of_import;
let __const_decl_ast =
  ("foo" |> of_public |> as_lexeme |> of_named_export, nil_prim |> of_const)
  |> of_decl;

let __ns_context =
  NamespaceContext.create(
    ~modules=
      [
        (
          "bar" |> of_internal,
          ModuleTable.{
            ast: [],
            types:
              [(Export.Main, Type2.Result.Valid(`String))]
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
  );

let suite =
  "Grammar.Program"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse import"
    >: (
      () =>
        [(__main_import, [__main_import_ast])]
        |> Assert.parse_many(~ns_context=__ns_context)
    ),
    "parse declaration"
    >: (
      () =>
        [
          (__const_decl, [__const_decl_ast]),
          (
            __const_decl ++ "; const bar = foo",
            [
              __const_decl_ast,
              (
                "bar" |> of_public |> as_lexeme |> of_named_export,
                "foo" |> of_public |> as_lexeme |> of_id |> as_nil |> of_const,
              )
              |> of_decl,
            ],
          ),
        ]
        |> Assert.parse_many
    ),
    "parse import and declaration"
    >: (
      () =>
        [
          (
            Print.fmt("%s; %s", __main_import, __const_decl),
            [__main_import_ast, __const_decl_ast],
          ),
        ]
        |> Assert.parse_many(~ns_context=__ns_context)
    ),
    "parse import with dependent declaration"
    >: (
      () =>
        Assert.parse(
          ~ns_context=
            NamespaceContext.create(
              ~modules=
                [
                  (
                    "bar" |> of_internal,
                    ModuleTable.{
                      ast: [],
                      types:
                        [(Export.Main, Type2.Result.Valid(`Boolean))]
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
          __main_import ++ "; const bar = foo",
          [
            __main_import_ast,
            (
              "bar" |> of_public |> as_lexeme |> of_named_export,
              "foo" |> of_public |> as_lexeme |> of_id |> as_bool |> of_const,
            )
            |> of_decl,
          ],
        )
    ),
    "parse imports only"
    >: (
      () =>
        [
          (__main_import |> Print.fmt("%s; gibberish"), [__main_import_ast]),
        ]
        |> AssertImports.parse_many(~ns_context=__ns_context)
    ),
  ];
