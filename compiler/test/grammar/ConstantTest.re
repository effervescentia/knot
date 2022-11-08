open Kore;

module A = AST.Result;
module Export = Reference.Export;
module ParseContext = AST.ParseContext;
module SymbolTable = AST.SymbolTable;
module T = AST.Type;
module U = Util.ResultUtil;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((A.export_t, A.declaration_t), unit);

    let parser = ctx =>
      KConstant.Plugin.parse(ctx, A.of_named_export)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) =>
              KDeclaration.Plugin.to_xml(~@AST.Type.pp, fst(stmt))
              |> Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Constant"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.parse_none(["gibberish", "const", "const foo", "const foo ="])
    ),
    "parse"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped |> A.of_named_export,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> U.as_untyped,
          "const foo = nil",
        )
    ),
    "parse with complex derived type"
    >: (
      () => {
        let symbols = {
          ...SymbolTable.create(),
          declared: {
            values: [
              ("bar", T.Valid(`Float)),
              ("fizz", T.Valid(`Integer)),
              ("buzz", T.Valid(`Float)),
            ],
            types: [],
          },
        };

        Assert.parse(
          ~context=
            ParseContext.create(
              ~symbols,
              Reference.Namespace.Internal("foo"),
            ),
          (
            "foo" |> U.as_untyped |> A.of_named_export,
            [
              (U.as_untyped("x"), "bar" |> A.of_id |> U.as_float)
              |> A.of_var
              |> U.as_nil,
              (
                U.as_untyped("y"),
                (
                  ("x" |> A.of_id |> U.as_float, "fizz" |> A.of_id |> U.as_int)
                  |> A.of_gt_op
                  |> U.as_bool,
                  (
                    "x" |> A.of_id |> U.as_float,
                    "buzz" |> A.of_id |> U.as_float,
                  )
                  |> A.of_ineq_op
                  |> U.as_bool,
                )
                |> A.of_and_op
                |> U.as_bool,
              )
              |> A.of_var
              |> U.as_nil,
              (
                "y" |> A.of_id |> U.as_bool,
                (
                  ("x" |> A.of_id |> U.as_float, 1 |> U.int_prim)
                  |> A.of_add_op
                  |> U.as_float,
                  5 |> U.int_prim,
                )
                |> A.of_lte_op
                |> U.as_bool,
              )
              |> A.of_or_op
              |> U.as_bool
              |> A.of_expr
              |> U.as_bool,
            ]
            |> A.of_closure
            |> U.as_bool
            |> A.of_const
            |> U.as_bool,
          )
          |> U.as_untyped,
          "const foo = {
            let x = bar;
            let y = x > fizz && x != buzz;
            y || x + 1 <= 5;
          }",
        );

        Assert.symbol_assoc_list(
          [
            ("bar", T.Valid(`Float)),
            ("fizz", T.Valid(`Integer)),
            ("buzz", T.Valid(`Float)),
            ("foo", T.Valid(`Boolean)),
          ],
          symbols.declared.values,
        );
      }
    ),
  ];
