open Knot.Kore;

module ViewKind = {
  type t =
    | Component
    | Element;

  /* methods */

  let to_string =
    fun
    | Component => "Component"
    | Element => "Element";
};

module Attribute = {
  type t('expr, 'typ) = (
    AST.Common.identifier_t,
    option(Node.t('expr, 'typ)),
  );

  type node_t('expr, 'typ) = AST.Common.raw_t(t('expr, 'typ));
};

module Child = {
  type t('expr, 'ksx, 'typ) =
    | Text(string)
    // TODO: change name to KSX
    | Node('ksx)
    | InlineExpression(Node.t('expr, 'typ));

  type node_t('expr, 'ksx, 'typ) = AST.Common.raw_t(t('expr, 'ksx, 'typ));

  /* static */

  let of_text = text => Text(text);
  let of_node = ksx => Node(ksx);
  let of_inline = expression => InlineExpression(expression);
};

type t('expr, 'typ) =
  | Tag(
      ViewKind.t,
      Node.t(string, 'typ),
      list(Node.t('expr, 'typ)),
      list(Attribute.node_t('expr, 'typ)),
      list(Child.node_t('expr, t('expr, 'typ), 'typ)),
    )
  | Fragment(list(Child.node_t('expr, t('expr, 'typ), 'typ)));

/* plugin types */

module Plugin =
  AST.Framework.Expression.MakeTypes({
    type parse_arg_t('ast, 'expr) = (
      AST.ParseContext.t('ast),
      (
        /* parses a "term" */
        AST.Framework.Interface.contextual_parse_t('ast, 'expr),
        /* parses an "expression" */
        AST.Framework.Interface.contextual_parse_t('ast, 'expr),
        /* parses a style literal */
        AST.Framework.Interface.contextual_parse_t('ast, 'expr),
      ),
    );
    type analyze_arg_t('ast, 'raw_expr, 'result_expr) =
      AST.Framework.Interface.analyze_t('ast, 'raw_expr, 'result_expr);
    type format_arg_t('expr, 'typ) = 'expr => bool;
    type value_t('expr, 'typ) = t('expr, 'typ);
  });

/* static */

let of_tag = ((kind, view, styles, attributes, children)) =>
  Tag(kind, view, styles, attributes, children);
let of_element_tag = ((view, styles, attributes, children)) =>
  (Element, view, styles, attributes, children) |> of_tag;
let of_component_tag = ((view, styles, attributes, children)) =>
  (Component, view, styles, attributes, children) |> of_tag;
let of_fragment = children => Fragment(children);
