open Kore;
open Util;
open Reference;

module Program = Grammar.Program;
module RawUtil = AST.Raw.Util;

module Target = {
  open AST.Raw;

  type t = program_t;

  let parser = Program.main % Parser.parse;

  let test =
    Alcotest.(
      check(
        list(
          testable(
            pp =>
              Debug.print_mod_stmt
              % Cow.Xml.list
              % Cow.Xml.to_string
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

    let parser = Program.imports % Parser.parse;
  });
module Assert = Assert.Make(Target);

let __main_import = "import foo from \"@/bar\"";
let __const_decl = "const foo = nil";
let __scope_tree = BinaryTree.create((Cursor.zero |> Cursor.expand, None));

let __main_import_ast =
  (
    "bar" |> RawUtil.internal,
    ["foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_import],
  )
  |> RawUtil.import;
let __const_decl_ast =
  (
    "foo" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
    nil_prim |> RawUtil.const,
  )
  |> RawUtil.decl;

let __scope =
  Scope.create(
    ~modules=
      [
        (
          "bar" |> RawUtil.internal,
          ModuleTable.{
            ast: [],
            types:
              [(Export.Main, Type.K_Strong(K_String))]
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
  );

let suite =
  "Grammar.Program"
  >::: [
    "no parse" >: (() => ["gibberish"] |> Assert.no_parse),
    "parse import"
    >: (
      () =>
        [(__main_import, [__main_import_ast])]
        |> Assert.parse_many(~scope=__scope)
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
                "bar" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
                "foo"
                |> RawUtil.public
                |> as_lexeme
                |> RawUtil.id
                |> as_nil
                |> RawUtil.const,
              )
              |> RawUtil.decl,
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
        |> Assert.parse_many(~scope=__scope)
    ),
    "parse import with dependent declaration"
    >: (
      () =>
        Assert.parse(
          ~scope=
            Scope.create(
              ~modules=
                [
                  (
                    "bar" |> RawUtil.internal,
                    ModuleTable.{
                      ast: [],
                      types:
                        [(Export.Main, Type.K_Strong(K_Boolean))]
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
          __main_import ++ "; const bar = foo",
          [
            __main_import_ast,
            (
              "bar" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
              "foo"
              |> RawUtil.public
              |> as_lexeme
              |> RawUtil.id
              |> as_bool
              |> RawUtil.const,
            )
            |> RawUtil.decl,
          ],
        )
    ),
    "parse imports only"
    >: (
      () =>
        [
          (__main_import |> Print.fmt("%s; gibberish"), [__main_import_ast]),
        ]
        |> AssertImports.parse_many(~scope=__scope)
    ),
  ];
