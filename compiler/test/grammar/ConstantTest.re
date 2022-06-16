open Kore;

module Export = Reference.Export;
module Constant = Grammar.Constant;
module U = Util.ResultUtil;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N2.t((A.export_t, A.declaration_t), unit);

    let parser = ((_, ctx)) =>
      Constant.parser(ctx, A.of_named_export)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) => {
              let (export, decl) = fst(stmt);

              A.Dump.(
                untyped_node_to_entity(
                  "Declaration",
                  ~children=[
                    export |> export_to_entity,
                    decl |> decl_to_entity,
                  ],
                  stmt,
                )
                |> Entity.pp(ppf)
              );
            },
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Declaration | Constant"
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
            "foo" |> A.of_public |> U.as_untyped |> A.of_named_export,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> U.as_untyped,
          "const foo = nil",
        )
    ),
    "parse with complex derived type"
    >: (
      () => {
        let declarations =
          [
            (Export.Named(A.of_public("bar")), T.Valid(`Float)),
            (Export.Named(A.of_public("fizz")), T.Valid(`Integer)),
            (Export.Named(A.of_public("buzz")), T.Valid(`Float)),
          ]
          |> List.to_seq
          |> DeclarationTable.of_seq;

        Assert.parse(
          ~mod_context=x => ModuleContext.create(~declarations, x),
          (
            "foo" |> A.of_public |> U.as_untyped |> A.of_named_export,
            [
              (
                "x" |> A.of_public |> U.as_untyped,
                "bar" |> A.of_public |> A.of_id |> U.as_float,
              )
              |> A.of_var
              |> U.as_nil,
              (
                "y" |> A.of_public |> U.as_untyped,
                (
                  (
                    "x" |> A.of_public |> A.of_id |> U.as_float,
                    "fizz" |> A.of_public |> A.of_id |> U.as_int,
                  )
                  |> A.of_gt_op
                  |> U.as_bool,
                  (
                    "x" |> A.of_public |> A.of_id |> U.as_float,
                    "buzz" |> A.of_public |> A.of_id |> U.as_float,
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
                "y" |> A.of_public |> A.of_id |> U.as_bool,
                (
                  (
                    "x" |> A.of_public |> A.of_id |> U.as_float,
                    1 |> U.int_prim,
                  )
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

        /* TODO: uncomment assertions */
        Assert.hashtbl(
          ~@Export.pp,
          ~@T.pp,
          [
            (Export.Named(A.of_public("bar")), T.Valid(`Float)),
            (Export.Named(A.of_public("fizz")), T.Valid(`Integer)),
            (Export.Named(A.of_public("buzz")), T.Valid(`Float)),
            (Export.Named(A.of_public("foo")), T.Valid(`Boolean)),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          declarations.scope,
        );
      }
    ),
  ];
