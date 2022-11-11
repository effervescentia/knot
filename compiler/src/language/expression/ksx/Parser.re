open Knot.Kore;
open Parse.Kore;
open AST;

module Character = Constants.Character;
module Glyph = Constants.Glyph;

module Tag = {
  let open_ = Matchers.symbol(Character.open_chevron);
  let close = Matchers.symbol(Character.close_chevron);

  let self_close = Matchers.glyph(Glyph.self_close_tag);
  let open_end = Matchers.glyph(Glyph.open_end_tag);
};

module Fragment = {
  let open_ = Matchers.glyph(Glyph.open_fragment);
  let close = Matchers.glyph(Glyph.close_fragment);
};

type expression_parsers_arg_t = (
  /* parses a "term" */
  ParserTypes.contextual_expression_parser_t,
  /* parses an "expression" */
  ParserTypes.contextual_expression_parser_t,
);

type jsx_parser_t = Parse.Parser.t(Node.t(Raw.jsx_t, unit));

type jsx_attribute_parser_t = Parse.Parser.t(Raw.jsx_attribute_t);
type jsx_attribute_list_parser_t = Parse.Parser.t(list(Raw.jsx_attribute_t));

type jsx_child_parser_t = Parse.Parser.t(Raw.jsx_child_t);
type jsx_child_list_parser_t = Parse.Parser.t(list(Raw.jsx_child_t));

let _attribute =
    (
      ~prefix=Matchers.alpha <|> Matchers.underscore,
      ctx: ParseContext.t,
      (parse_term, parse_expr): expression_parsers_arg_t,
    ) =>
  Matchers.assign(
    Matchers.identifier(~prefix),
    parse_expr(ctx)
    |> Matchers.between_parentheses
    >|= (
      ((expr, _) as expr_group) =>
        Node.map_range(_ => Node.get_range(expr_group), expr)
    )
    <|> parse_term(ctx),
  )
  >|= (
    ((name, value)) => (name, Some(value), Node.join_ranges(name, value))
  )
  <|> (
    Matchers.identifier(~prefix) >|= (id => (id, None, Node.get_range(id)))
  );

let _self_closing = Tag.self_close >|= Node.map(() => []);

let rec _inner_ksx =
        (ctx: ParseContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  /* do not attempt to simplify this `input` argument away or JSX parsing will loop forever */
  input =>
    (choice([fragment(ctx, parsers), tag(ctx, parsers)]) |> Matchers.lexeme)(
      input,
    )

and fragment =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  children(ctx, parsers)
  |> Matchers.between(Fragment.open_, Fragment.close)
  >|= Node.map(Raw.of_frag)

and tag =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  Tag.open_
  >> Matchers.identifier
  >>= (
    id =>
      attributes(ctx, parsers)
      >>= (
        attrs =>
          Tag.close
          >> children(ctx, parsers)
          >>= (
            cs =>
              cs
              <$| (
                id
                |> fst
                |> Matchers.keyword
                |> Matchers.between(Tag.open_end, Tag.close)
              )
          )
          <|> _self_closing
          >|= (
            cs =>
              Node.untyped(
                (id, attrs, fst(cs)) |> Raw.of_tag,
                Node.join_ranges(id, cs),
              )
          )
      )
  )

and property_attribute =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_parser_t =>
  _attribute(ctx, parsers)
  >|= (
    ((name, value, range)) =>
      Node.untyped((name, value) |> Raw.of_prop, range)
  )

and class_attribute =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_parser_t =>
  _attribute(~prefix=Matchers.period, ctx, parsers)
  >|= (
    ((name, value, range)) =>
      Node.untyped(
        (name |> Node.map(String.drop_left(1)), value) |> Raw.of_jsx_class,
        range,
      )
  )

and id_attribute: jsx_attribute_parser_t =
  Matchers.identifier(~prefix=char(Character.octothorpe))
  >|= Node.map(String.drop_left(1))
  >|= Node.wrap(Raw.of_jsx_id)

and attributes =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_list_parser_t =>
  choice([
    property_attribute(ctx, parsers),
    class_attribute(ctx, parsers),
    id_attribute,
  ])
  |> many

and children =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_list_parser_t =>
  choice([node(ctx, parsers), inline_expr(ctx, parsers), text])
  |> Matchers.lexeme
  |> many

and text: jsx_child_parser_t =
  none_of(. Character.[open_brace, open_chevron])
  <~> (
    none_of(. Character.[open_brace, open_chevron, close_chevron]) |> many
  )
  >|= Input.join
  >|= Node.map(String.trim % Raw.of_text)

and node =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_parser_t =>
  _inner_ksx(ctx, parsers) >|= Node.map(Raw.of_node)

and inline_expr =
    (ctx: ParseContext.t, (_, parse_expr): expression_parsers_arg_t)
    : jsx_child_parser_t =>
  parse_expr(ctx) |> Matchers.between_braces >|= Node.map(Raw.of_inline_expr);

let ksx =
    ((ctx: ParseContext.t, parsers: expression_parsers_arg_t))
    : ParserTypes.expression_parser_t =>
  _inner_ksx(ctx, parsers) >|= Node.map(Raw.of_jsx);
