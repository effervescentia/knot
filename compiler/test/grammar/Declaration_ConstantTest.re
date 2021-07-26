open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = (export_t, declaration_t);

    let parser = ((_, ctx, _)) =>
      Parser.parse(Declaration.constant(ctx, AST.of_named_export));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Debug.print_decl % Pretty.to_string % Format.pp_print_string(pp),
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
              "foo" |> of_public |> as_lexeme |> of_named_export,
              nil_prim |> of_const,
            ),
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
          |> DefinitionTable.from_seq;

        Assert.parse(
          ~mod_context=x => ModuleContext.create(~definitions, x),
          "const foo = {
            let x = bar;
            let y = x > fizz && x != buzz;
            y || x + 1 <= 5;
          }",
          (
            "foo" |> of_public |> as_lexeme |> of_named_export,
            [
              (
                "x" |> of_public |> as_lexeme,
                "bar" |> of_public |> as_lexeme |> of_id |> as_float,
              )
              |> of_var,
              (
                "y" |> of_public |> as_lexeme,
                (
                  (
                    "x" |> of_public |> as_lexeme |> of_id |> as_float,
                    "fizz" |> of_public |> as_lexeme |> of_id |> as_int,
                  )
                  |> of_gt_op
                  |> as_bool,
                  (
                    "x" |> of_public |> as_lexeme |> of_id |> as_float,
                    "buzz" |> of_public |> as_lexeme |> of_id |> as_float,
                  )
                  |> of_ineq_op
                  |> as_bool,
                )
                |> of_and_op
                |> as_bool,
              )
              |> of_var,
              (
                "y" |> of_public |> as_lexeme |> of_id |> as_bool,
                (
                  (
                    "x" |> of_public |> as_lexeme |> of_id |> as_float,
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
              |> of_expr,
            ]
            |> of_closure
            |> as_bool
            |> of_const,
          ),
        );

        /* Assert.int(0, scope.seed^); */
        Assert.hashtbl(
          Export.to_string,
          Type.to_string,
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
             | Ok(t) => t |> Type.trait_to_string |> Print.fmt("Ok(%s)")
             | Error((x, y)) =>
               Print.fmt(
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
