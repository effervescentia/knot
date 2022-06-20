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
    primitive_types |> List.map(((kwd, prim)) => kwd >|= N.map(_ => prim)),
  );

let group = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> M.between(Symbol.open_group, Symbol.close_group)
  >|= N.map(TE.of_group);

let list = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Glyph.list_type_suffix
       >|= (
         (suffix, expr) =>
           N.untyped(TE.of_list(expr), N.join_ranges(expr, suffix))
       ),
     );

let struct_ = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  M.identifier(~prefix=M.alpha)
  >>= (id => Symbol.colon >> parse_expr >|= Tuple.with_fst2(id))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  /* TODO: sort the props here by property name */
  >|= N.map(props => TE.of_struct(props));

let function_ =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> M.comma_sep
  |> M.between(Symbol.open_group, Symbol.close_group)
  >>= (
    args =>
      Glyph.lambda
      >> parse_expr
      >|= (
        res =>
          N.untyped(
            TE.of_function((fst(args), res)),
            N.join_ranges(args, res),
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
  >|= N.get_range
  >>= (
    start =>
      M.identifier(~prefix=M.alpha)
      >>= (
        id =>
          Symbol.colon
          >> parser
          >|= (
            ((res, range)) =>
              N.untyped((id, res) |> f, Range.join(start, range |?: start))
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
      |> option(id |> N.map(_ => []))
      >|= N.map(Tuple.with_fst2(id))
  );

let type_variants = (ctx: ModuleContext.t) =>
  optional(Symbol.vertical_bar)
  >> (_type_variant(ctx) |> sep_by(Symbol.vertical_bar));

let declaration: type_module_statement_parser_t =
  ctx =>
    _module_statement(
      Keyword.decl,
      expression_parser
      >|= (expr => (expr, expr |> N.get_range |> Option.some)),
      TD.of_declaration,
    );

let enumerated: type_module_statement_parser_t =
  ctx =>
    _module_statement(
      Keyword.enum,
      type_variants(
        ModuleContext.create(
          NamespaceContext.create(Reference.Namespace.Ambient),
        ),
      )
      >|= (
        variants => {
          let variant_range = variants |> List.last |?> N.get_range;

          (variants |> List.map(fst), variant_range);
        }
      ),
      TD.of_enum,
    );

let type_: type_module_statement_parser_t =
  ctx =>
    _module_statement(
      Keyword.type_,
      expression_parser
      >|= (expr => (expr, expr |> N.get_range |> Option.some)),
      TD.of_type,
    );

let module_statement: type_module_statement_parser_t =
  ctx =>
    choice([declaration(ctx), enumerated(ctx), type_(ctx)])
    >@= fst
    % (
      fun
      | TD.Enumerated((id, _), raw_variants) => {
          let variants =
            raw_variants
            |> List.map(
                 Tuple.map_each2(
                   fst,
                   List.map(
                     fst
                     % Analyze.Typing.eval_type_expression(ctx.definitions),
                   ),
                 ),
               );
          let enum_type = T.Valid(`Enumerated(variants));
          let value_type =
            T.Valid(
              `Struct(
                variants
                |> List.map(
                     Tuple.map_snd2(args =>
                       T.Valid(`Function((args, enum_type)))
                     ),
                   ),
              ),
            );

          ctx.definitions |> DefinitionTable.define_type(id, enum_type);
          ctx.definitions |> DefinitionTable.define_value(id, value_type);
        }

      | TD.Type((id, _), (expr, _)) => {
          let type_ =
            expr |> Analyze.Typing.eval_type_expression(ctx.definitions);

          ctx.definitions |> DefinitionTable.define_type(id, type_);
        }

      | TD.Declaration((id, _), (expr, _)) => {
          let type_ =
            expr |> Analyze.Typing.eval_type_expression(ctx.definitions);

          ctx.definitions |> DefinitionTable.define_value(id, type_);
        }
    )
    |> M.terminated;

let module_parser: type_module_parser_t =
  ctx =>
    Keyword.module_
    >>= (
      _ =>
        M.identifier(~prefix=M.alpha)
        >>= (
          id => {
            let module_ctx =
              ModuleContext2.create(Inner(fst(id), None), ctx);

            module_statement(module_ctx)
            |> many
            |> M.between(Symbol.open_closure, Symbol.close_closure)
            >|= (
              stmts =>
                N.untyped(
                  (id, fst(stmts)) |> TD.of_module,
                  N.get_range(stmts),
                )
            );
          }
        )
    );
