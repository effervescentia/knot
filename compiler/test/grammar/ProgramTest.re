open Kore;
open Reference;

module Program = Grammar.Program;
module U = Util.ResultUtil;

module Target = {
  type t = A.program_t;

  let parser = Program.main % Parser.parse;

  let test =
    Alcotest.(
      check(
        list(
          testable(
            ppf => A.Dump.(mod_stmt_to_entity % Entity.pp(ppf)),
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
let __scope_tree = BinaryTree.create((Range.zero, None));

let __main_import_ast =
  (
    "bar" |> A.of_internal,
    ["foo" |> U.as_untyped |> A.of_main_import |> U.as_untyped],
  )
  |> A.of_import
  |> U.as_untyped;
let __const_decl_ast =
  (
    "foo" |> U.as_untyped |> A.of_named_export,
    U.nil_prim |> A.of_const |> U.as_nil,
  )
  |> A.of_decl
  |> U.as_untyped;

let __context =
  ParseContext.create(
    ~modules=
      [
        (
          "bar" |> A.of_internal,
          ModuleTable.Valid(
            "foo",
            {
              ast: [],
              exports:
                [(Export.Main, Type.Valid(`String))]
                |> List.to_seq
                |> Hashtbl.of_seq,
              scopes: __scope_tree,
            },
          ),
        ),
      ]
      |> List.to_seq
      |> Hashtbl.of_seq,
    Internal("mock"),
  );

let suite =
  "Grammar.Program"
  >::: [
    "no parse" >: (() => Assert.no_parse("gibberish")),
    "parse import"
    >: (
      () =>
        Assert.parse(~context=__context, [__main_import_ast], __main_import)
    ),
    "parse single declaration"
    >: (() => Assert.parse([__const_decl_ast], __const_decl)),
    "parse multiple declarations"
    >: (
      () =>
        Assert.parse(
          [
            __const_decl_ast,
            (
              "bar" |> U.as_untyped |> A.of_named_export,
              "foo" |> A.of_id |> U.as_nil |> A.of_const |> U.as_nil,
            )
            |> A.of_decl
            |> U.as_untyped,
          ],
          __const_decl ++ "; const bar = foo",
        )
    ),
    "parse import and declaration"
    >: (
      () =>
        Assert.parse(
          ~context=__context,
          [__main_import_ast, __const_decl_ast],
          Fmt.str("%s; %s", __main_import, __const_decl),
        )
    ),
    "parse import with dependent declaration"
    >: (
      () =>
        Assert.parse(
          ~context=
            ParseContext.create(
              ~modules=
                [
                  (
                    "bar" |> A.of_internal,
                    ModuleTable.Valid(
                      "foo",
                      {
                        ast: [],
                        exports:
                          [(Export.Main, Type.Valid(`Boolean))]
                          |> List.to_seq
                          |> Hashtbl.of_seq,
                        scopes: __scope_tree,
                      },
                    ),
                  ),
                ]
                |> List.to_seq
                |> Hashtbl.of_seq,
              Internal("mock"),
            ),
          [
            __main_import_ast,
            (
              "bar" |> U.as_untyped |> A.of_named_export,
              "foo" |> A.of_id |> U.as_bool |> A.of_const |> U.as_bool,
            )
            |> A.of_decl
            |> U.as_untyped,
          ],
          __main_import ++ "; const bar = foo",
        )
    ),
    "parse imports only"
    >: (
      () =>
        AssertImports.parse(
          ~context=__context,
          [__main_import_ast],
          __main_import |> Fmt.str("%s; gibberish"),
        )
    ),
  ];
