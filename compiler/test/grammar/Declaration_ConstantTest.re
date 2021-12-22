open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = Node.Raw.t((export_t, declaration_t));

    let parser = ((_, ctx)) =>
      Parser.parse(Declaration.constant(ctx, AST.of_named_export));

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) => {
              let (export, decl) = Node.Raw.get_value(stmt);

              Dump.raw_node_to_entity(
                "Declaration",
                ~children=[
                  export |> Dump.export_to_entity,
                  decl |> Dump.decl_to_entity,
                ],
                stmt,
              )
              |> Dump.Entity.pp(ppf);
            },
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Declaration (Constant)"
  >::: [
    "no parse"
    >: (
      () =>
        ["gibberish", "const", "const foo", "const foo ="] |> Assert.no_parse
    ),
    "parse"
    >: (
      () =>
        [
          (
            "const foo = nil",
            (
              "foo" |> of_public |> as_raw_node |> of_named_export,
              nil_prim |> of_const |> as_nil,
            )
            |> as_raw_node,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse with complex derived type"
    >: (
      () => {
        let definitions =
          Type.[
            (Export.Named(of_public("bar")), Valid(`Float)),
            (Export.Named(of_public("fizz")), Valid(`Integer)),
            (Export.Named(of_public("buzz")), Valid(`Float)),
          ]
          |> List.to_seq
          |> DefinitionTable.of_seq;

        Assert.parse(
          ~mod_context=x => ModuleContext.create(~definitions, x),
          "const foo = {
            let x = bar;
            let y = x > fizz && x != buzz;
            y || x + 1 <= 5;
          }",
          (
            "foo" |> of_public |> as_raw_node |> of_named_export,
            [
              (
                "x" |> of_public |> as_raw_node,
                "bar" |> of_public |> as_float |> of_id |> as_float,
              )
              |> of_var
              |> as_nil,
              (
                "y" |> of_public |> as_raw_node,
                (
                  (
                    "x" |> of_public |> as_float |> of_id |> as_float,
                    "fizz" |> of_public |> as_int |> of_id |> as_int,
                  )
                  |> of_gt_op
                  |> as_bool,
                  (
                    "x" |> of_public |> as_float |> of_id |> as_float,
                    "buzz" |> of_public |> as_float |> of_id |> as_float,
                  )
                  |> of_ineq_op
                  |> as_bool,
                )
                |> of_and_op
                |> as_bool,
              )
              |> of_var
              |> as_bool,
              (
                "y" |> of_public |> as_bool |> of_id |> as_bool,
                (
                  (
                    "x" |> of_public |> as_float |> of_id |> as_float,
                    1 |> int_prim,
                  )
                  |> of_add_op
                  |> as_float,
                  5 |> int_prim,
                )
                |> of_lte_op
                |> as_bool,
              )
              |> of_or_op
              |> as_bool
              |> of_expr
              |> as_bool,
            ]
            |> of_closure
            |> as_bool
            |> of_const
            |> as_bool,
          )
          |> as_raw_node,
        );

        /* TODO: uncomment assertions */
        /* Assert.int(0, scope.seed^); */
        Assert.hashtbl(
          ~@Export.pp,
          ~@Type.pp,
          Type.[
            (Export.Named(of_public("bar")), Valid(`Float)),
            (Export.Named(of_public("fizz")), Valid(`Integer)),
            (Export.Named(of_public("buzz")), Valid(`Float)),
            (Export.Named(of_public("foo")), Valid(`Boolean)),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          definitions.scope,
        );
        /* Assert.hashtbl(
             string_of_int,
             fun
             | Ok(t) => t |> Type.trait_to_string |> Fmt.str("Ok(%s)")
             | Error((x, y)) =>
               Fmt.str(
                 "Error(%s, %s)",
                 x |> Type.trait_to_string,
                 y |> Type.trait_to_string,
               ),
             [] |> List.to_seq |> Hashtbl.of_seq,
             scope.anonymous,
           ); */
      }
    ),
  ];
