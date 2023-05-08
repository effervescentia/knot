open Kore;

module U = Util.ResultUtil;

let suite =
  "Grammar.Constant"
  >::: [
    "no parse"
    >: (
      () =>
        Assert.Declaration.parse_none([
          "gibberish",
          "const",
          "const foo",
          "const foo =",
        ])
    ),
    "parse"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            U.nil_prim |> Declaration.of_constant |> U.as_nil,
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
              ("bar", Valid(Float)),
              ("fizz", Valid(Integer)),
              ("buzz", Valid(Float)),
            ],
            types: [],
          },
        };

        Assert.Declaration.parse(
          ~context=
            ParseContext.create(
              ~symbols,
              Reference.Namespace.Internal("foo"),
            ),
          (
            U.as_untyped("foo"),
            [
              (
                U.as_untyped("x"),
                "bar" |> Expression.of_identifier |> U.as_float,
              )
              |> Statement.of_variable
              |> U.as_nil,
              (
                U.as_untyped("y"),
                (
                  (
                    "x" |> Expression.of_identifier |> U.as_float,
                    "fizz" |> Expression.of_identifier |> U.as_int,
                  )
                  |> Expression.of_gt_op
                  |> U.as_bool,
                  (
                    "x" |> Expression.of_identifier |> U.as_float,
                    "buzz" |> Expression.of_identifier |> U.as_float,
                  )
                  |> Expression.of_unequal_op
                  |> U.as_bool,
                )
                |> Expression.of_and_op
                |> U.as_bool,
              )
              |> Statement.of_variable
              |> U.as_nil,
              (
                "y" |> Expression.of_identifier |> U.as_bool,
                (
                  (
                    "x" |> Expression.of_identifier |> U.as_float,
                    1 |> U.int_prim,
                  )
                  |> Expression.of_add_op
                  |> U.as_float,
                  5 |> U.int_prim,
                )
                |> Expression.of_lte_op
                |> U.as_bool,
              )
              |> Expression.of_or_op
              |> U.as_bool
              |> Statement.of_effect
              |> U.as_bool,
            ]
            |> Expression.of_closure
            |> U.as_bool
            |> Declaration.of_constant
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
            ("bar", Valid(Float)),
            ("fizz", Valid(Integer)),
            ("buzz", Valid(Float)),
            ("foo", Valid(Boolean)),
          ],
          symbols.declared.values,
        );
      }
    ),
  ];
