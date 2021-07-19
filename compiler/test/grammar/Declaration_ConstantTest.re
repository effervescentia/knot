open Kore;
open AST.Raw.Util;
open Reference;
open Util;

module Declaration = Grammar.Declaration;

module Assert = {
  open AST.Raw;

  include Assert;
  include Assert.Make({
    type t = (export_t, declaration_t);

    let parser = ctx =>
      Parser.parse(Declaration.constant(ctx, to_named_export));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Debug.print_decl
              % Cow.Xml.to_string
              % Format.pp_print_string(pp),
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
              "foo" |> to_public |> as_lexeme |> to_named_export,
              nil_prim |> to_const,
            ),
          ),
        ]
        |> Assert.parse_many
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
            "foo" |> to_public |> as_lexeme |> to_named_export,
            [
              ("x" |> to_public |> as_lexeme, "bar" |> to_public_id(as_float))
              |> to_var,
              (
                "y" |> to_public |> as_lexeme,
                (
                  (
                    "x" |> to_public_id(as_float),
                    "fizz" |> to_public_id(as_int),
                  )
                  |> to_gt_op
                  |> as_bool,
                  (
                    "x" |> to_public_id(as_float),
                    "buzz" |> to_public_id(as_float),
                  )
                  |> to_ineq_op
                  |> as_bool,
                )
                |> to_and_op
                |> as_bool,
              )
              |> to_var,
              (
                "y" |> to_public_id(as_bool),
                (
                  ("x" |> to_public_id(as_float), 1 |> int_prim)
                  |> to_add_op
                  |> as_float,
                  5 |> int_prim,
                )
                |> to_lte_op
                |> as_bool,
              )
              |> to_or_op
              |> as_bool
              |> to_expr,
            ]
            |> to_closure
            |> as_bool
            |> to_const,
          ),
        );

        Assert.int(0, scope.seed^);
        Assert.hashtbl(
          Export.to_string,
          Type.to_string,
          [
            (Export.Named("bar" |> to_public), Type.K_Strong(K_Float)),
            (Export.Named("fizz" |> to_public), Type.K_Strong(K_Integer)),
            (Export.Named("buzz" |> to_public), Type.K_Strong(K_Float)),
            (Export.Named("foo" |> to_public), Type.K_Strong(K_Boolean)),
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
