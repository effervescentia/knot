open Knot.Kore;

module Parser = Parse.Parser;

type unary_op_parser_t = Parser.t(Raw.expression_t => Raw.expression_t);

type binary_op_parser_t =
  Parser.t(((Raw.expression_t, Raw.expression_t)) => Raw.expression_t);

type expression_parser_t = Parser.t(Raw.expression_t);
type contextual_expression_parser_t = ParseContext.t => expression_parser_t;

type statement_parser_t = Parser.t(Raw.statement_t);

type declaration_parser_t =
  Parser.t(Node.t((Common.identifier_t, Module.declaration_t), unit));

module type PrimitiveParams = {
  type value_t;

  let parse: Parser.t(Node.t(value_t, unit));

  let format: Fmt.t(value_t);

  let to_xml: value_t => Fmt.xml_t(string);
};

module Primitive = (Params: PrimitiveParams) => {
  include Params;
};

module type StatementParams = {
  type value_t('expr, 'typ);

  let parse:
    ((ParseContext.t, contextual_expression_parser_t)) =>
    Parser.t(Common.raw_t(value_t(Raw.expression_t, unit)));

  let format:
    Fmt.t(Result.raw_expression_t) =>
    Fmt.t(value_t(Result.expression_t, Type.t));

  let to_xml:
    ('expr => Fmt.xml_t(string), value_t('expr, 'typ)) => Fmt.xml_t(string);
};

module Statement = (Params: StatementParams) => {
  include Params;
};

module type NoParseExpressionParams = {
  type value_t('expr, 'typ);

  let format:
    Fmt.t(Result.raw_expression_t) =>
    Fmt.t(value_t(Expression.raw_expression_t(Type.t), Type.t));

  let to_xml:
    (
      (Node.t('expr, 'typ) => Fmt.xml_t(string), 'typ => string),
      value_t('expr, 'typ)
    ) =>
    Fmt.xml_t(string);
};

module NoParseExpression = (Params: NoParseExpressionParams) => {
  include Params;
};

module type ExpressionParams = {
  include NoParseExpressionParams;

  type parse_arg_t;

  let parse: parse_arg_t => Parser.t(Expression.expression_t(unit));
};

module Expression = (Params: ExpressionParams) => {
  include Params;
};

module type DeclarationParams = {
  type value_t;

  let parse:
    ((ParseContext.t, Module.export_kind_t)) =>
    Parser.t(Node.t((Common.identifier_t, Result.node_t(value_t)), unit));

  let format: Fmt.t((string, value_t));

  let to_xml: (Type.t => string, value_t) => Fmt.xml_t(string);
};

module Declaration = (Params: DeclarationParams) => {
  include Params;
};
