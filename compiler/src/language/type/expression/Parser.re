open Knot.Kore;
open Parse.Kore;
open AST;

module Keyword = Constants.Keyword;

type type_expression_parser_t = Parse.Parser.t(Interface.node_t);

let parse_primitive: type_expression_parser_t =
  choice(
    Interface.[
      Nil <$| Matchers.keyword(Keyword.nil),
      Boolean <$| Matchers.keyword(Keyword.boolean),
      Integer <$| Matchers.keyword(Keyword.integer),
      Float <$| Matchers.keyword(Keyword.float),
      String <$| Matchers.keyword(Keyword.string),
      Element <$| Matchers.keyword(Keyword.element),
      Style <$| Matchers.keyword(Keyword.style),
    ],
  );

let parse_group =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr |> Matchers.between_parentheses >|= Node.map(Interface.of_group);

let parse_list =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Matchers.glyph("[]")
       >|= (
         (suffix, expr) =>
           Node.raw(Interface.of_list(expr), Node.join_ranges(expr, suffix))
       ),
     );

let __object_key = Matchers.identifier(~prefix=Matchers.alpha);

let parse_required_object_property = parse_expr =>
  Matchers.attribute(__object_key, parse_expr)
  >|= (
    ((key, value)) =>
      Node.raw(
        (key, value) |> Interface.ObjectEntry.of_required,
        Node.join_ranges(key, value),
      )
  );

let parse_optional_object_property = parse_expr =>
  Matchers.binary_op(
    __object_key,
    Matchers.glyph(Constants.Glyph.conditional),
    parse_expr,
  )
  >|= (
    ((key, value)) =>
      Node.raw(
        (key, value) |> Interface.ObjectEntry.of_optional,
        Node.join_ranges(key, value),
      )
  );

let parse_spread_properties = parse_expr =>
  Matchers.glyph(Constants.Glyph.spread)
  >>= (
    keyword =>
      parse_expr
      >|= (
        expr =>
          Node.raw(
            Interface.ObjectEntry.of_spread(expr),
            Node.join_ranges(keyword, expr),
          )
      )
  );

let parse_object =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  choice([
    parse_spread_properties(parse_expr),
    parse_optional_object_property(parse_expr),
    parse_required_object_property(parse_expr),
  ])
  |> Matchers.comma_sep
  |> Matchers.between_braces
  /* TODO: sort the props here by property name */
  >|= Node.map(Interface.of_object);

let parse_function =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> Matchers.comma_sep
  |> Matchers.between_parentheses
  >>= (
    args =>
      Matchers.glyph("->")
      >> parse_expr
      >|= (
        res =>
          Node.raw(
            Interface.of_function((fst(args), res)),
            Node.join_ranges(args, res),
          )
      )
  );

let parse_identifier: type_expression_parser_t =
  Matchers.identifier >|= Node.wrap(Interface.of_identifier);

let parse_dot_access = {
  let rec loop = expr =>
    Matchers.period
    >> Matchers.identifier
    >>= (
      prop =>
        loop(
          Node.raw(
            (expr, prop) |> Interface.of_dot_access,
            Node.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};

let parse_view =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  Matchers.keyword(Keyword.view)
  >>= (
    start =>
      parse_expr
      >>= (
        props =>
          Matchers.symbol(Constants.Character.comma)
          >> parse_expr
          >|= Tuple.with_fst2(props)
      )
      |> Matchers.between_parentheses
      >|= fst
      >|= (
        ((props, res)) =>
          Node.raw(
            (props, res) |> Interface.of_view,
            Node.join_ranges(start, res),
          )
      )
  );

/*
  each type expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* element[], float[][][] */
let rec parse_expression_0: type_expression_parser_t =
  input => (parse_list(parse_expression_1))(input)

/* foo.bar */
and parse_expression_1: type_expression_parser_t =
  input => (parse_expression_2 >>= parse_dot_access)(input)

/* nil, (string), (integer, float) -> boolean, { foo: string } */
and parse_expression_2: type_expression_parser_t =
  input =>
    choice(
      [
        parse_view(parse_expression_0),
        parse_function(parse_expression_0),
        parse_group(parse_expression_0),
        parse_object(parse_expression_0),
        parse_primitive,
        parse_identifier,
      ],
      input,
    );

let parse = parse_expression_0;
