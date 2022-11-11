open Knot.Kore;

type unary_op_parser_t = Parse.Parser.t(Raw.expression_t => Raw.expression_t);

type binary_op_parser_t =
  Parse.Parser.t(((Raw.expression_t, Raw.expression_t)) => Raw.expression_t);

type expression_parser_t = Parse.Parser.t(Raw.expression_t);
type contextual_expression_parser_t = ParseContext.t => expression_parser_t;

type statement_parser_t = Parse.Parser.t(Raw.statement_t);

type declaration_parser_t =
  Parse.Parser.t(Node.t((Module.export_t, Module.declaration_t), unit));
