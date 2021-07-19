open Kore;
open Util;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  open AST.Raw;

  include Assert;
  include Assert.Make({
    type t = (export_t, declaration_t);

    let parser = ctx =>
      Parser.parse(Declaration.constant(ctx, RawUtil.named_export));

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

let _public_id = f => RawUtil.public % as_lexeme % RawUtil.id % f;

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
              "foo" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
              nil_prim |> RawUtil.const,
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
            "foo" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
            [
              (
                "x" |> RawUtil.public |> as_lexeme,
                "bar" |> _public_id(as_float),
              )
              |> RawUtil.var,
              (
                "y" |> RawUtil.public |> as_lexeme,
                (
                  ("x" |> _public_id(as_float), "fizz" |> _public_id(as_int))
                  |> RawUtil.gt_op
                  |> as_bool,
                  (
                    "x" |> _public_id(as_float),
                    "buzz" |> _public_id(as_float),
                  )
                  |> RawUtil.ineq_op
                  |> as_bool,
                )
                |> RawUtil.and_op
                |> as_bool,
              )
              |> RawUtil.var,
              (
                "y" |> _public_id(as_bool),
                (
                  ("x" |> _public_id(as_float), 1 |> int_prim)
                  |> RawUtil.add_op
                  |> as_float,
                  5 |> int_prim,
                )
                |> RawUtil.lte_op
                |> as_bool,
              )
              |> RawUtil.or_op
              |> as_bool
              |> RawUtil.expr,
            ]
            |> RawUtil.closure
            |> as_bool
            |> RawUtil.const,
          ),
        );

        Assert.int(0, scope.seed^);
        Assert.hashtbl(
          Export.to_string,
          Type.to_string,
          [
            (Export.Named("bar" |> RawUtil.public), Type.K_Strong(K_Float)),
            (
              Export.Named("fizz" |> RawUtil.public),
              Type.K_Strong(K_Integer),
            ),
            (
              Export.Named("buzz" |> RawUtil.public),
              Type.K_Strong(K_Float),
            ),
            (
              Export.Named("foo" |> RawUtil.public),
              Type.K_Strong(K_Boolean),
            ),
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
