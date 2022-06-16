open Kore;

module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;

let primitive_types =
  TE.[
    (Keyword.nil, Nil),
    (Keyword.boolean, Boolean),
    (Keyword.integer, Integer),
    (Keyword.float, Float),
    (Keyword.string, String),
    (Keyword.element, Element),
  ];

let primitive: type_expression_parser_t =
  choice(
    primitive_types
    |> List.map(((kwd, prim)) => kwd >|= NR.map_value(_ => prim)),
  );

let group = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> M.between(Symbol.open_group, Symbol.close_group)
  >|= NR.map_value(TE.of_group);

let list = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Glyph.list_type_suffix
       >|= (
         (suffix, expr) =>
           NR.create(TE.of_list(expr), NR.join_ranges(expr, suffix))
       ),
     );

let struct_ = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  M.identifier(~prefix=M.alpha)
  >>= (id => Symbol.colon >> parse_expr >|= Tuple.with_fst2(id))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  /* TODO: sort the props here by property name */
  >|= NR.map_value(props => TE.of_struct(props));

let function_ =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> M.comma_sep
  |> M.between(Symbol.open_group, Symbol.close_group)
  >>= (
    ((args, args_range)) =>
      Glyph.lambda
      >> parse_expr
      >|= (
        res =>
          NR.create(
            TE.of_function((args, res)),
            Range.join(args_range, NR.get_range(res)),
          )
      )
  );

/*
  each type expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* element[], float[][][] */
let rec expr_0: type_expression_parser_t = input => (list(expr_1))(input)

/* nil, (string), (integer, float) -> boolean, { foo: string } */
and expr_1: type_expression_parser_t =
  input =>
    choice(
      [function_(expr_0), group(expr_0), struct_(expr_0), primitive],
      input,
    );

let expression_parser = expr_0;

let _module_statement = (kwd, parser, f) =>
  kwd
  >|= NR.get_range
  >>= (
    start =>
      M.identifier(~prefix=M.alpha)
      >>= (
        id =>
          Symbol.colon
          >> parser
          >|= (
            ((res, range)) =>
              NR.create((id, res) |> f, Range.join(start, range |?: start))
          )
      )
  );

let _type_variant = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      expression_parser
      |> M.comma_sep
      |> M.between(Symbol.open_group, Symbol.close_group)
      |> option(([], NR.get_range(id)))
      >|= NR.map_value(Tuple.with_fst2(id))
  );

let type_variants = (ctx: ModuleContext.t) =>
  optional(Symbol.vertical_bar)
  >> (_type_variant(ctx) |> sep_by(Symbol.vertical_bar));

let declaration: type_module_statement_parser_t =
  _module_statement(
    Keyword.decl,
    expression_parser
    >|= (expr => (expr, expr |> NR.get_range |> Option.some)),
    TD.of_declaration,
  );

let enumerated: type_module_statement_parser_t =
  _module_statement(
    Keyword.enum,
    type_variants(
      ModuleContext.create(
        NamespaceContext.create(Reference.Namespace.Ambient),
      ),
    )
    >|= (
      variants => {
        let variant_range = variants |> List.last |?> NR.get_range;

        (
          variants
          |> List.map(
               NR.get_value
               % Tuple.map_fst2(NR.get_value % Reference.Identifier.to_string),
             ),
          variant_range,
        );
      }
    ),
    TD.of_enum,
  );

let type_: type_module_statement_parser_t =
  _module_statement(
    Keyword.type_,
    expression_parser
    >|= (expr => (expr, expr |> NR.get_range |> Option.some)),
    TD.of_type,
  );

let module_statement: type_module_statement_parser_t =
  choice([declaration, enumerated, type_]) |> M.terminated;

let module_parser: type_module_parser_t =
  Keyword.module_
  >|= NR.get_range
  >>= (
    start =>
      M.identifier(~prefix=M.alpha)
      >>= (
        id =>
          module_statement
          |> many
          |> M.between(Symbol.open_closure, Symbol.close_closure)
          >|= (
            ((stmts, range)) =>
              NR.create((id, stmts) |> TD.of_module, range)
          )
      )
  );
