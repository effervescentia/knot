open Kore;

module U = Util.RawUtil;
module ObjectEntry = TypeExpression.ObjectEntry;

let __empty_defs = AST.SymbolTable.create();

let suite =
  "Analyze.Typing | Type Expression"
  >::: [
    "nil"
    >: (
      () =>
        Assert.type_(
          Valid(Nil),
          KTypeExpression.Plugin.analyze(__empty_defs, Nil),
        )
    ),
    "boolean"
    >: (
      () =>
        Assert.type_(
          Valid(Boolean),
          KTypeExpression.Plugin.analyze(__empty_defs, Boolean),
        )
    ),
    "integer"
    >: (
      () =>
        Assert.type_(
          Valid(Integer),
          KTypeExpression.Plugin.analyze(__empty_defs, Integer),
        )
    ),
    "float"
    >: (
      () =>
        Assert.type_(
          Valid(Float),
          KTypeExpression.Plugin.analyze(__empty_defs, Float),
        )
    ),
    "string"
    >: (
      () =>
        Assert.type_(
          Valid(String),
          KTypeExpression.Plugin.analyze(__empty_defs, String),
        )
    ),
    "element"
    >: (
      () =>
        Assert.type_(
          Valid(Element),
          KTypeExpression.Plugin.analyze(__empty_defs, Element),
        )
    ),
    "identifier"
    >: (
      () => {
        let symbols = AST.SymbolTable.create();

        symbols.declared.types =
          symbols.declared.types @ [("foo", Valid(Boolean))];

        Assert.type_(
          Valid(Boolean),
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
          Valid(String),
          TypeExpression.String
          |> U.as_untyped
          |> TypeExpression.of_group
          |> U.as_untyped
          |> TypeExpression.of_group
          |> U.as_untyped
          |> TypeExpression.of_group
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "list type"
    >: (
      () =>
        Assert.type_(
          Valid(List(Valid(Boolean))),
          TypeExpression.Boolean
          |> U.as_untyped
          |> TypeExpression.of_list
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "struct type"
    >: (
      () =>
        Assert.type_(
          Valid(
            Object([
              ("foo", (Valid(Boolean), true)),
              ("bar", (Valid(String), false)),
            ]),
          ),
          [
            (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            (U.as_untyped("bar"), U.as_untyped(TypeExpression.String))
            |> ObjectEntry.of_optional
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
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
                Object([
                  ("foo", (Valid(Element), false)),
                  ("fizz", (Valid(Boolean), false)),
                ]),
              ),
            ),
            (
              "var2",
              Valid(
                Object([
                  ("buzz", (Valid(Integer), true)),
                  ("bar", (Valid(Boolean), true)),
                ]),
              ),
            ),
          ];

        Assert.type_(
          Valid(
            Object([
              ("foo", (Valid(Element), false)),
              ("fizz", (Valid(Boolean), false)),
              ("buzz", (Valid(Integer), true)),
              ("bar", (Valid(String), false)),
            ]),
          ),
          [
            (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
            |> ObjectEntry.of_required
            |> U.as_untyped,
            U.as_untyped(TypeExpression.Identifier(U.as_untyped("var1")))
            |> ObjectEntry.of_spread
            |> U.as_untyped,
            U.as_untyped(
              TypeExpression.Object([
                (U.as_untyped("buzz"), U.as_untyped(TypeExpression.Float))
                |> ObjectEntry.of_optional
                |> U.as_untyped,
                U.as_untyped(
                  TypeExpression.Identifier(U.as_untyped("var2")),
                )
                |> ObjectEntry.of_spread
                |> U.as_untyped,
              ]),
            )
            |> ObjectEntry.of_spread
            |> U.as_untyped,
            (U.as_untyped("bar"), U.as_untyped(TypeExpression.String))
            |> ObjectEntry.of_optional
            |> U.as_untyped,
          ]
          |> TypeExpression.of_object
          |> KTypeExpression.Plugin.analyze(symbols),
        );
      }
    ),
    "function type"
    >: (
      () =>
        Assert.type_(
          Valid(
            Function([Valid(Boolean), Valid(String)], Valid(Element)),
          ),
          (
            [
              U.as_untyped(TypeExpression.Boolean),
              U.as_untyped(TypeExpression.String),
            ],
            U.as_untyped(TypeExpression.Element),
          )
          |> TypeExpression.of_function
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "view type"
    >: (
      () =>
        Assert.type_(
          Valid(
            View(
              [
                ("foo", (Valid(Boolean), true)),
                ("bar", (Valid(String), true)),
              ],
              Valid(Element),
            ),
          ),
          (
            [
              (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
              |> ObjectEntry.of_required
              |> U.as_untyped,
              (U.as_untyped("bar"), U.as_untyped(TypeExpression.String))
              |> ObjectEntry.of_required
              |> U.as_untyped,
            ]
            |> TypeExpression.of_object
            |> U.as_untyped,
            U.as_untyped(TypeExpression.Element),
          )
          |> TypeExpression.of_view
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
    "invalid view properties type"
    >: (
      () =>
        Assert.type_(
          Invalid(NotInferrable),
          (
            U.as_untyped(TypeExpression.Boolean),
            U.as_untyped(TypeExpression.Element),
          )
          |> TypeExpression.of_view
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
              (U.as_untyped("foo"), U.as_untyped(TypeExpression.Boolean))
              |> ObjectEntry.of_required
              |> U.as_untyped,
              (U.as_untyped("bar"), U.as_untyped(TypeExpression.String))
              |> ObjectEntry.of_required
              |> U.as_untyped,
            ]
            |> TypeExpression.of_object
            |> U.as_untyped,
            U.as_untyped(TypeExpression.Style),
          )
          |> TypeExpression.of_view
          |> KTypeExpression.Plugin.analyze(__empty_defs),
        )
    ),
  ];
