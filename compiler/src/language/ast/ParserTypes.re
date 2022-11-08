open Knot.Kore;

type number_parser_t = Parse.Kore.parser_t(Node.t(Raw.number_t, unit));

type primitive_parser_t = Parse.Kore.parser_t(Node.t(Raw.primitive_t, unit));

type unary_op_parser_t =
  Parse.Kore.parser_t(Raw.expression_t => Raw.expression_t);

type binary_op_parser_t =
  Parse.Kore.parser_t(
    ((Raw.expression_t, Raw.expression_t)) => Raw.expression_t,
  );

type expression_parser_t = Parse.Kore.parser_t(Raw.expression_t);
type contextual_expression_parser_t = ParseContext.t => expression_parser_t;

type statement_parser_t = Parse.Kore.parser_t(Raw.statement_t);

type declaration_parser_t =
  Parse.Kore.parser_t(
    Node.t((Result.export_t, Result.declaration_t), unit),
  );
