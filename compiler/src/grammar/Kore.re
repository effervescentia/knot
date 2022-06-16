include Knot.Kore;
include Parse.Onyx;
include ModuleAliases;

module C = Constants;
module M = Matchers;

type stream_t = LazyStream.t(Input.t);
type parser_t('a) = stream_t => option(('a, stream_t));

/* concrete parser types */

type number_parser_t = parser_t(N2.t(AR.number_t, TR.t));

type string_parser_t = parser_t(N2.t(string, unit));

type primitive_parser_t = parser_t(N2.t(AR.primitive_t, TR.t));

type identifier_parser_t = parser_t(AR.identifier_t);

type jsx_parser_t = parser_t(N2.t(AR.jsx_t, unit));

type jsx_attribute_parser_t = parser_t(AR.jsx_attribute_t);
type jsx_attribute_list_parser_t = parser_t(list(AR.jsx_attribute_t));

type jsx_child_parser_t = parser_t(AR.jsx_child_t);
type jsx_child_list_parser_t = parser_t(list(AR.jsx_child_t));

type expression_parser_t = parser_t(AR.expression_t);
type contextual_expression_parser_t = ModuleContext.t => expression_parser_t;

type expression_parsers_arg_t = (
  /* parses a "term" */
  contextual_expression_parser_t,
  /* parses an "expression" */
  contextual_expression_parser_t,
);

type statement_parser_t = parser_t(AR.statement_t);

type argument_list_parser_t = parser_t(list(AR.argument_t));
type lambda_parser_t =
  parser_t((list(AR.argument_t), AR.expression_t, Range.t));

type import_parser_t = parser_t(list(A.import_t));

type declaration_parser_t =
  parser_t(N2.t((A.export_t, A.declaration_t), unit));

type type_expression_parser_t = parser_t(AST.TypeExpression.t);

type type_module_statement_parser_t =
  parser_t(AST.TypeDefinition.module_statement_t);

type type_module_parser_t =
  TypingNamespaceContext.t => parser_t(AST.TypeDefinition.module_t);
