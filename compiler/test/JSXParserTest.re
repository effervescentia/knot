open Core;

let structure_props =
  List.fold_left(
    (acc, (key, value)) => acc @ [Identifier(key), Assign] @ value,
    [],
  );

let self_closed_jsx = (~props=[], name) =>
  [LeftChevron, Identifier(name)]
  @ structure_props(props)
  @ [JSXSelfClose]
  |> Util.drift;
let container_jsx = (~props=[], name) =>
  [LeftChevron, Identifier(name)]
  @ structure_props(props)
  @ [
    RightChevron,
    LeftChevron,
    Identifier("somechild"),
    JSXSelfClose,
    JSXOpenEnd,
    Identifier(name),
    RightChevron,
  ]
  |> Util.drift;
let container_with_exprs_jsx = name =>
  [LeftChevron, Identifier(name)]
  @ [
    RightChevron,
    LeftBrace,
    Identifier("A"),
    Asterisk,
    Identifier("B"),
    Minus,
    Identifier("C"),
    RightBrace,
    LeftChevron,
    Identifier("innerchild"),
    JSXSelfClose,
    LeftBrace,
    Identifier("f"),
    LeftParenthese,
    Number(8),
    RightParenthese,
    RightBrace,
    JSXOpenEnd,
    Identifier(name),
    RightChevron,
  ]
  |> Util.drift;
let fragment_jsx = names =>
  [JSXStartFragment]
  @ List.fold_left(
      (acc, name) => acc @ [LeftChevron, Identifier(name), JSXSelfClose],
      [],
      names,
    )
  @ [JSXEndFragment]
  |> Util.drift;

let test_parse_jsx =
  Util.test_parse_jsx(KnotParse.JSX.expr(KnotParse.Expression.expr));

let __name = "MyComponent";

let tests =
  "KnotParse.JSX"
  >::: [
    "parse with reserved name"
    >:: (
      _ =>
        test_parse_jsx((
          self_closed_jsx("import"),
          Element("import", [], []),
        ))
    ),
    "parse self closing"
    >:: (
      _ =>
        test_parse_jsx((self_closed_jsx(__name), Element(__name, [], [])))
    ),
    "parse self closing with params"
    >:: (
      _ =>
        test_parse_jsx((
          self_closed_jsx(
            __name,
            ~props=[
              ("abc", [String("def")]),
              ("hij", [LeftBrace, Number(332), RightBrace]),
            ],
          ),
          Element(
            __name,
            [("abc", StringLit("def")), ("hij", NumericLit(332))],
            [],
          ),
        ))
    ),
    "parse container with child"
    >:: (
      _ =>
        test_parse_jsx((
          container_jsx(
            __name,
            ~props=[
              (
                "expr",
                [
                  LeftBrace,
                  Identifier("A"),
                  Plus,
                  Identifier("B"),
                  RightBrace,
                ],
              ),
              ("bool", [LeftBrace, Boolean(false), RightBrace]),
            ],
          ),
          Element(
            __name,
            [
              (
                "expr",
                AddExpr(
                  Reference(Variable("A")),
                  Reference(Variable("B")),
                ),
              ),
              ("bool", BooleanLit(false)),
            ],
            [Element("somechild", [], [])],
          ),
        ))
    ),
    "parse container with child and props"
    >:: (
      _ =>
        test_parse_jsx((
          container_jsx(__name),
          Element(__name, [], [Element("somechild", [], [])]),
        ))
    ),
    "parse container with child and injected expressions"
    >:: (
      _ =>
        test_parse_jsx((
          container_with_exprs_jsx(__name),
          Element(
            __name,
            [],
            [
              EvalNode(
                SubExpr(
                  MulExpr(
                    Reference(Variable("A")),
                    Reference(Variable("B")),
                  ),
                  Reference(Variable("C")),
                ),
              ),
              Element("innerchild", [], []),
              EvalNode(
                Reference(Execution(Variable("f"), [NumericLit(8)])),
              ),
            ],
          ),
        ))
    ),
    "parse children in fragment"
    >:: (
      _ => {
        let child_2 = "otherchild";
        let child_3 = "otherchild";

        test_parse_jsx((
          fragment_jsx([__name, child_2, child_3]),
          Fragment([
            Element(__name, [], []),
            Element(child_2, [], []),
            Element(child_3, [], []),
          ]),
        ));
      }
    ),
  ];
