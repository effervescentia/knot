open Knot.Kore;

module Parser = Parse.Parser;

module type NumberParams = {
  type value_t;

  let parse: Parser.t(Node.t(Raw.number_t, unit));

  let pp: Fmt.t(value_t);

  let to_xml: value_t => Fmt.xml_t(string);
};

module Number = (Params: NumberParams) => {
  include Params;
};

module type PrimitiveParams = {
  type value_t;

  let parse: Parser.t(Node.t(Raw.primitive_t, unit));

  let pp: Fmt.t(value_t);

  let to_xml: value_t => Fmt.xml_t(string);
};

module Primitive = (Params: PrimitiveParams) => {
  include Params;
};

module type StatementParams = {
  type pp_arg_t;

  type value_t('a);

  let parse:
    ((ParseContext.t, ParserTypes.contextual_expression_parser_t)) =>
    Parser.t(Raw.statement_t);

  let pp: Fmt.t(Result.raw_expression_t) => Fmt.t(value_t(Type.t));

  let to_xml:
    (Expression.expression_t('a) => Fmt.xml_t(string), value_t('a)) =>
    Fmt.xml_t(string);
};

module Statement = (Params: StatementParams) => {
  include Params;
};

module type NoParseExpressionParams = {
  // type analyze_arg_t;

  type value_t('a);

  // let analyze: (analyze_arg_t, value_t(unit)) => value_t(Type.t);
  let pp: Fmt.t(Result.raw_expression_t) => Fmt.t(value_t(Type.t));

  let to_xml:
    (
      (Expression.expression_t('a) => Fmt.xml_t(string), 'a => string),
      value_t('a)
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
    ((ParseContext.t, Node.t(string, unit) => Module.export_t)) =>
    Parser.t(Node.t((Module.export_t, Module.declaration_t), unit));

  let pp: Fmt.t((string, value_t));

  let to_xml: (Type.t => string, value_t) => Fmt.xml_t(string);
};

module Declaration = (Params: DeclarationParams) => {
  include Params;
};
