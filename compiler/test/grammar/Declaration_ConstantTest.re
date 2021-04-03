open Kore;
open AST;
open Util;

module Declaration = Grammar.Declaration;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = (identifier_t, declaration_t);

    let parser = scope =>
      Parser.parse(Declaration.constant(Context.create(~scope, ())));

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
        Assert.parse(
          "const foo = nil",
          ("foo" |> of_public |> as_lexeme, nil_prim |> of_const),
        )
    ),
    "parse with complex derived type"
    >: (
      () => {
        let scope =
          to_scope([
            ("bar", K_Strong(K_Float)),
            ("fizz", K_Strong(K_Integer)),
            ("buzz", K_Strong(K_Float)),
          ]);

        Assert.parse(
          ~scope,
          "const foo = {
            let x = bar;
            let y = x > fizz && x != buzz;
            y || x + 1 <= 5;
          }",
          (
            "foo" |> of_public |> as_lexeme,
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

        Assert.int(0, scope.seed^);
        Assert.hashtbl(
          Reference.Identifier.to_string,
          Type.to_string,
          [
            ("bar" |> of_public, Type.K_Strong(K_Float)),
            ("fizz" |> of_public, Type.K_Strong(K_Integer)),
            ("buzz" |> of_public, Type.K_Strong(K_Float)),
            ("foo" |> of_public, Type.K_Strong(K_Boolean)),
          ]
          |> List.to_seq
          |> Hashtbl.of_seq,
          scope.types,
        );
        Assert.hashtbl(
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
        );
      }
    ),
  ];
