open Knot.Kore;
open Parse.Kore;

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

type parse_ksx_t('expr) =
  Parse.Parser.t(Node.t(Interface.t('expr, unit), unit));

type parse_ksx_attribute_t('expr) =
  Parse.Parser.t(Interface.Attribute.node_t('expr, unit));
type parse_ksx_attribute_list_t('expr) =
  Parse.Parser.t(list(Interface.Attribute.node_t('expr, unit)));

type parse_ksx_child_t('expr) =
  Parse.Parser.t(
    Interface.Child.node_t('expr, Interface.t('expr, unit), unit),
  );
type parse_ksx_child_list_t('expr) =
  Parse.Parser.t(
    list(Interface.Child.node_t('expr, Interface.t('expr, unit), unit)),
  );

let _parse_self_closing = Tag.self_close >|= Node.map(() => []);

let _parse_style_binding =
    (
      (ctx, (parse_term, _, parse_style)):
        Interface.Plugin.parse_arg_t('ast, 'expr),
    ) =>
  Matchers.glyph(Glyph.style_binding)
  >> (parse_style(ctx) <|> parse_term(ctx));

let _parse_attribute =
    (
      ~prefix=Matchers.alpha <|> Matchers.underscore,
      (ctx, (parse_term, parse_expression, parse_style)):
        Interface.Plugin.parse_arg_t('ast, 'expr),
    )
    : parse_ksx_attribute_t('expr) =>
  Matchers.assign(
    Matchers.identifier(~prefix),
    parse_expression(ctx)
    |> Matchers.between_parentheses
    >|= (
      ((expression, _) as expr_group) =>
        Node.map_range(_ => Node.get_range(expr_group), expression)
    )
    <|> parse_term(ctx),
  )
  >|= (
    ((name, expression)) =>
      Node.raw(
        (name, Some(expression)),
        Node.join_ranges(name, expression),
      )
  )
  <|> (
    Matchers.identifier(~prefix)
    >|= (name => Node.raw((name, None), Node.get_range(name)))
  );

let parse_inline =
    ((ctx, (_, parse_expression, _))): parse_ksx_child_t('expr) =>
  parse_expression(ctx)
  |> Matchers.between_braces
  >|= Node.map(Interface.Child.of_inline);

let parse_text: parse_ksx_child_t('expr) =
  none_of(Character.[open_brace, open_chevron])
  <~> (none_of(Character.[open_brace, open_chevron, close_chevron]) |> many)
  >|= Input.join
  >|= Node.map(String.trim % Interface.Child.of_text);

let rec _parse_inner_ksx = (arg): parse_ksx_t('expr) =>
  /* do not attempt to simplify this `input` argument away or JSX parsing will loop forever */
  input =>
    (choice([parse_fragment(arg), parse_tag(arg)]) |> Matchers.lexeme)(
      input,
    )

and parse_fragment = (arg): parse_ksx_t('expr) =>
  parse_children(arg)
  |> Matchers.between(Fragment.open_, Fragment.close)
  >|= Node.map(Interface.of_fragment)

and parse_tag = (arg): parse_ksx_t('expr) =>
  Tag.open_
  >> Matchers.identifier
  >>= (
    id =>
      _parse_style_binding(arg)
      |> many
      >>= (
        styles =>
          _parse_attribute(arg)
          |> many
          >>= (
            attributes =>
              Tag.close
              >> parse_children(arg)
              >>= (
                children =>
                  children
                  <$| (
                    id
                    |> fst
                    |> Matchers.keyword
                    |> Matchers.between(Tag.open_end, Tag.close)
                  )
              )
              <|> _parse_self_closing
              >|= (
                children =>
                  Node.raw(
                    (id, styles, attributes, fst(children))
                    |> Interface.of_element_tag,
                    Node.join_ranges(id, children),
                  )
              )
          )
      )
  )

and parse_children = (arg): parse_ksx_child_list_t('expr) =>
  choice([parse_node(arg), parse_inline(arg), parse_text])
  |> Matchers.lexeme
  |> many

and parse_node = (arg): parse_ksx_child_t('expr) =>
  _parse_inner_ksx(arg) >|= Node.map(Interface.Child.of_node);

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, arg) => _parse_inner_ksx(arg) >|= Node.map(f);
