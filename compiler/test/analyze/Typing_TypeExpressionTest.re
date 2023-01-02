open Kore;

module TE = AST.TypeExpression;
module U = Util.RawUtil;

let __empty_defs = AST.SymbolTable.create();

let suite =
  "Analyze.Typing | Type Expression"
  >::: [
    "nil"
    >: (
      () =>
        Assert.type_(
          Valid(`Nil),
          KTypeExpression.Plugin.analyze(__empty_defs, Nil),
        )
    ),
    "boolean"
    >: (
      () =>
        Assert.type_(
          Valid(`Boolean),
          KTypeExpression.Plugin.analyze(__empty_defs, Boolean),
        )
    ),
    "integer"
    >: (
      () =>
        Assert.type_(
          Valid(`Integer),
          KTypeExpression.Plugin.analyze(__empty_defs, Integer),
        )
    ),
    "float"
    >: (
      () =>
        Assert.type_(
          Valid(`Float),
          KTypeExpression.Plugin.analyze(__empty_defs, Float),
        )
    ),
    "string"
    >: (
      () =>
        Assert.type_(
          Valid(`String),
          KTypeExpression.Plugin.analyze(__empty_defs, String),
        )
    ),
    "element"
    >: (
      () =>
        Assert.type_(
          Valid(`Element),
          KTypeExpression.Plugin.analyze(__empty_defs, Element),
        )
    ),
    "identifier"
    >: (
      () => {
        let symbols = AST.SymbolTable.create();

        symbols.declared.types =
          symbols.declared.types @ [("foo", Valid(`Boolean))];

        Assert.type_(
          Valid(`Boolean),
          KTypeExpression.Plugin.analyze(
            symbols,
            Identifier(U.as_untyped("foo")),
          ),
        );
      }
    ),
    "grouped type"
    >: (
      () =>
        Assert.type_(
          Valid(`String),
          TE.String
          |> U.as_untyped
          |> TE.of_group
          |> U.as_untyped
          |> TE.of_group
          |> U.as_untyped
          |> TE.of_group
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "list type"
    >: (
      () =>
        Assert.type_(
          Valid(`List(Valid(`Boolean))),
          TE.Boolean
          |> U.as_untyped
          |> TE.of_list
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "struct type"
    >: (
      () =>
        Assert.type_(
          Valid(
            `Object([
              ("foo", (Valid(`Boolean), true)),
              ("bar", (Valid(`String), false)),
            ]),
          ),
          [
            (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
            |> TE.of_required
            |> U.as_untyped,
            (U.as_untyped("bar"), U.as_untyped(TE.String))
            |> TE.of_optional
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "struct type with spread and overrides"
    >: (
      () => {
        let symbols = AST.SymbolTable.create();

        symbols.declared.types =
          symbols.declared.types
          @ [
            (
              "var1",
              Valid(
                `Object([
                  ("foo", (Valid(`Element), false)),
                  ("fizz", (Valid(`Boolean), false)),
                ]),
              ),
            ),
            (
              "var2",
              Valid(
                `Object([
                  ("buzz", (Valid(`Integer), true)),
                  ("bar", (Valid(`Boolean), true)),
                ]),
              ),
            ),
          ];

        Assert.type_(
          Valid(
            `Object([
              ("foo", (Valid(`Element), false)),
              ("fizz", (Valid(`Boolean), false)),
              ("buzz", (Valid(`Integer), true)),
              ("bar", (Valid(`String), false)),
            ]),
          ),
          [
            (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
            |> TE.of_required
            |> U.as_untyped,
            U.as_untyped(TE.Identifier(U.as_untyped("var1")))
            |> TE.of_spread
            |> U.as_untyped,
            U.as_untyped(
              TE.Object([
                (U.as_untyped("buzz"), U.as_untyped(TE.Float))
                |> TE.of_optional
                |> U.as_untyped,
                U.as_untyped(TE.Identifier(U.as_untyped("var2")))
                |> TE.of_spread
                |> U.as_untyped,
              ]),
            )
            |> TE.of_spread
            |> U.as_untyped,
            (U.as_untyped("bar"), U.as_untyped(TE.String))
            |> TE.of_optional
            |> U.as_untyped,
          ]
          |> TE.of_object
          |> KTypeExpression.Plugin.analyze(symbols),
        );
      }
    ),
    "function type"
    >: (
      () =>
        Assert.type_(
          Valid(
            `Function((
              [Valid(`Boolean), Valid(`String)],
              Valid(`Element),
            )),
          ),
          (
            [U.as_untyped(TE.Boolean), U.as_untyped(TE.String)],
            U.as_untyped(TE.Element),
          )
          |> TE.of_function
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "view type"
    >: (
      () =>
        Assert.type_(
          Valid(
            `View((
              [
                ("foo", (Valid(`Boolean), true)),
                ("bar", (Valid(`String), true)),
              ],
              Valid(`Element),
            )),
          ),
          (
            [
              (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
              |> TE.of_required
              |> U.as_untyped,
              (U.as_untyped("bar"), U.as_untyped(TE.String))
              |> TE.of_required
              |> U.as_untyped,
            ]
            |> TE.of_object
            |> U.as_untyped,
            U.as_untyped(TE.Element),
          )
          |> TE.of_view
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "invalid view properties type"
    >: (
      () =>
        Assert.type_(
          Invalid(NotInferrable),
          (U.as_untyped(TE.Boolean), U.as_untyped(TE.Element))
          |> TE.of_view
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "invalid view result type"
    >: (
      () =>
        Assert.type_(
          Invalid(NotInferrable),
          (
            [
              (U.as_untyped("foo"), U.as_untyped(TE.Boolean))
              |> TE.of_required
              |> U.as_untyped,
              (U.as_untyped("bar"), U.as_untyped(TE.String))
              |> TE.of_required
              |> U.as_untyped,
            ]
            |> TE.of_object
            |> U.as_untyped,
            U.as_untyped(TE.Style),
          )
          |> TE.of_view
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
  ];
