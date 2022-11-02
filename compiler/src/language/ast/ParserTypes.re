open Knot.Kore;

type number_parser_t = Parse.Kore.parser_t(Node.t(Raw.number_t, unit));

type string_parser_t = Parse.Kore.parser_t(Node.t(string, unit));

type primitive_parser_t = Parse.Kore.parser_t(Node.t(Raw.primitive_t, unit));

type identifier_parser_t = Parse.Kore.parser_t(Raw.identifier_t);

type jsx_parser_t = Parse.Kore.parser_t(Node.t(Raw.jsx_t, unit));

type jsx_attribute_parser_t = Parse.Kore.parser_t(Raw.jsx_attribute_t);
type jsx_attribute_list_parser_t =
  Parse.Kore.parser_t(list(Raw.jsx_attribute_t));

type jsx_child_parser_t = Parse.Kore.parser_t(Raw.jsx_child_t);
type jsx_child_list_parser_t = Parse.Kore.parser_t(list(Raw.jsx_child_t));

type expression_parser_t = Parse.Kore.parser_t(Raw.expression_t);
type contextual_expression_parser_t = ParseContext.t => expression_parser_t;

type expression_parsers_arg_t = (
  /* parses a "term" */
  contextual_expression_parser_t,
  /* parses an "expression" */
  contextual_expression_parser_t,
);

type statement_parser_t = Parse.Kore.parser_t(Raw.statement_t);

type argument_list_parser_t = Parse.Kore.parser_t(list(Raw.argument_t));
type lambda_parser_t =
  Parse.Kore.parser_t((list(Raw.argument_t), Raw.expression_t, Range.t));

type import_parser_t = Parse.Kore.parser_t(list(Result.import_t));

type declaration_parser_t =
  Parse.Kore.parser_t(
    Node.t((Result.export_t, Result.declaration_t), unit),
  );

type type_expression_parser_t = Parse.Kore.parser_t(TypeExpression.t);

type type_module_statement_parser_t =
  ParseContext.t => Parse.Kore.parser_t(TypeDefinition.module_statement_t);

type type_module_parser_t =
  ParseContext.t => Parse.Kore.parser_t(TypeDefinition.module_t);
