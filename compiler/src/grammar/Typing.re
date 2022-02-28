open Kore;

module TE = ASTV2.TypeExpression;

let primitive_types =
  TE.[
    (Keyword.nil, Nil),
    (Keyword.boolean, Boolean),
    (Keyword.integer, Integer),
    (Keyword.float, Float),
    (Keyword.string, String),
    (Keyword.element, Element),
  ];

let primitive: type_expression_parser_t =
  choice(
    primitive_types
    |> List.map(((kwd, prim)) => kwd >|= NR.map_value(_ => prim)),
  );

let group = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> M.between(Symbol.open_group, Symbol.close_group)
  >|= NR.map_value(TE.of_group);

let list = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Glyph.list_type_suffix
       >|= (
         (suffix, expr) =>
           NR.create(TE.of_list(expr), NR.join_ranges(expr, suffix))
       ),
     );

let struct_ = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  M.identifier(~prefix=M.alpha)
  >>= (id => Symbol.colon >> parse_expr >|= (expr => (id, expr)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  /* TODO: sort the props here by property name */
  >|= NR.map_value(props => TE.of_struct(props));

let function_ =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> M.comma_sep
  |> M.between(Symbol.open_group, Symbol.close_group)
  >>= (
    ((args, args_range)) =>
      Glyph.lambda
      >> parse_expr
      >|= (
        res =>
          NR.create(
            TE.of_function((args, res)),
            Range.join(args_range, NR.get_range(res)),
          )
      )
  );

/*
  each type expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* element[], float[][][] */
let rec expr_0: type_expression_parser_t = input => (list(expr_1))(input)

/* nil, (string), (integer, float) -> boolean, { foo: string } */
and expr_1: type_expression_parser_t =
  input =>
    choice(
      [function_(expr_0), group(expr_0), struct_(expr_0), primitive],
      input,
    );

let expression_parser = expr_0;
