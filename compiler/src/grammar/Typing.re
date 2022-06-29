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

let identifier: type_expression_parser_t = M.identifier >|= N.wrap(TE.of_id);

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
      [
        function_(expr_0),
        group(expr_0),
        struct_(expr_0),
        primitive,
        identifier,
      ],
      input,
    );

let expression_parser = expr_0;

let _define_statement = (kwd, parser, f) =>
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

let _type_variant = (ctx: ParseContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      expression_parser
      |> M.comma_sep
      |> M.between(Symbol.open_group, Symbol.close_group)
      |> option(id |> N.map(_ => []))
      >|= N.map(Tuple.with_fst2(id))
  );

let type_variants = (ctx: ParseContext.t) =>
  optional(Symbol.vertical_bar)
  >> (_type_variant(ctx) |> sep_by(Symbol.vertical_bar));

let declaration: type_module_statement_parser_t =
  ctx =>
    _define_statement(
      Keyword.declare,
      expression_parser
      >|= (expr => (expr, expr |> N.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ =
          raw_expr |> Analyze.Typing.eval_type_expression(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_value(id, type_);

        TD.of_declaration(res);
      },
    );

let enumerated: type_module_statement_parser_t =
  ctx =>
    _define_statement(
      Keyword.enum,
      type_variants(ctx)
      >|= (
        variants => {
          let variant_range = variants |> List.last |?> N.get_range;

          (variants |> List.map(fst), variant_range);
        }
      ),
      (((id, _), raw_variants) as res) => {
        let variants =
          raw_variants
          |> List.map(
               Tuple.map_each2(
                 fst,
                 List.map(
                   fst % Analyze.Typing.eval_type_expression(ctx.symbols),
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

        ctx.symbols |> SymbolTable.declare_type(id, enum_type);
        ctx.symbols |> SymbolTable.declare_value(id, value_type);

        TD.of_enum(res);
      },
    );

let type_: type_module_statement_parser_t =
  ctx =>
    _define_statement(
      Keyword.type_,
      expression_parser
      >|= (expr => (expr, expr |> N.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ =
          raw_expr |> Analyze.Typing.eval_type_expression(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_type(id, type_);

        TD.of_type(res);
      },
    );

let module_statement: type_module_statement_parser_t =
  ctx =>
    choice([declaration(ctx), enumerated(ctx), type_(ctx)]) |> M.terminated;

let _module_decorator = (ctx: ParseContext.t) =>
  Decorator.parser(Primitive.parser)
  >|= N.map(((id, args)) =>
        (
          id |> N.add_type(ctx.symbols |> SymbolTable.resolve_value(fst(id))),
          args |> List.map(N.map_type(Type.of_raw)),
        )
      );

let module_: type_module_parser_t =
  ctx =>
    _module_decorator(ctx)
    |> many
    |> option([])
    >>= (
      raw_decorators =>
        Keyword.module_
        >> (
          M.identifier(~prefix=M.alpha)
          >>= (
            id => {
              let module_ctx = ParseContext.create_module(ctx);

              module_statement(module_ctx)
              |> many
              |> M.between(Symbol.open_closure, Symbol.close_closure)
              >|= (
                stmts => {
                  let SymbolTable.Symbols.{types, values} =
                    module_ctx.symbols.declared;

                  if (!List.is_empty(types)) {
                    ctx.symbols
                    |> SymbolTable.declare_type(
                         fst(id),
                         Valid(
                           `Module(
                             types
                             |> List.map(
                                  Tuple.map_snd2(type_ =>
                                    Type.Container.Type(type_)
                                  ),
                                ),
                           ),
                         ),
                       );
                  };

                  if (!List.is_empty(values)) {
                    ctx.symbols
                    |> SymbolTable.declare_value(
                         fst(id),
                         Valid(
                           `Module(
                             values
                             |> List.map(
                                  Tuple.map_snd2(type_ =>
                                    Type.Container.Value(type_)
                                  ),
                                ),
                           ),
                         ),
                       );
                  };

                  let decorators =
                    raw_decorators
                    |> List.map(
                         Analyze.Semantic.analyze_decorator(ctx, Module),
                       );

                  N.untyped(
                    (id, fst(stmts), decorators) |> TD.of_module,
                    N.get_range(stmts),
                  );
                }
              );
            }
          )
        )
    );

let decorator: type_module_parser_t =
  ctx =>
    _define_statement(
      Keyword.decorator,
      expression_parser
      |> M.comma_sep
      |> M.between(Symbol.open_group, Symbol.close_group)
      >>= (
        args =>
          Keyword.on
          >> Keyword.module_
          >|= N.get_range
          % (range => ((fst(args), T.DecoratorTarget.Module), Some(range)))
      )
      |> M.terminated,
      ((id, (args, target))) => {
        let arg_types =
          args
          |> List.map(fst % Analyze.Typing.eval_type_expression(ctx.symbols));
        let type_ = T.Valid(`Decorator((arg_types, target)));

        ctx.symbols |> SymbolTable.declare_value(fst(id), type_);

        (id, args, target) |> TD.of_decorator;
      },
    );

let root_parser: type_module_parser_t =
  ctx => choice([decorator(ctx), module_(ctx)]);
