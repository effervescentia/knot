open AST;
open Kore;
open Parse.Kore;

type type_module_parser_t('ast) =
  ParseContext.t('ast) => Parse.Parser.t(Interface.node_t);

let __module_keyword = Matchers.keyword(Constants.Keyword.module_);

let _parse_module_decorator = (ctx: ParseContext.t('ast)) =>
  KDecorator.Plugin.parse(Primitive.parse_primitive)
  >|= Node.map(((id, args)) =>
        (
          id
          |> Node.add_type(
               ctx.symbols |> SymbolTable.resolve_value(fst(id)),
             ),
          args
          |> List.map(arg =>
               arg |> Node.analyzer(Primitive.analyze_primitive) % fst
             ),
        )
      );

let parse_module: type_module_parser_t('ast) =
  ctx =>
    _parse_module_decorator(ctx)
    |> many
    >>= (
      decorators =>
        __module_keyword
        >> (
          Matchers.identifier(~prefix=Matchers.alpha)
          >>= (
            id => {
              let module_ctx = ParseContext.create_module(ctx);

              TypeStatement.parse(module_ctx)
              |> many
              |> Matchers.between_braces
              >|= (
                statements => {
                  let SymbolTable.Symbols.{types, values} =
                    module_ctx.symbols.declared;
                  let module_types =
                    types
                    |> List.map(((name, value)) =>
                         (Type.ModuleEntryKind.Type, name, value)
                       );
                  let module_values =
                    values
                    |> List.map(((name, value)) =>
                         (Type.ModuleEntryKind.Value, name, value)
                       );
                  let has_types = !List.is_empty(module_types);
                  let has_values = !List.is_empty(module_values);

                  let decorators' =
                    decorators
                    |> List.map(Analyzer.analyze_decorator(ctx, Module));
                  let valid_decorators =
                    decorators'
                    |> List.filter_map(
                         fun
                         | ((((id, _), args), _), true) =>
                           Some((id, args |> List.map(fst)))
                         | _ => None,
                       );

                  if (has_types) {
                    ctx.symbols
                    |> SymbolTable.declare_type(
                         fst(id),
                         Valid(Module(module_types)),
                       );
                  };

                  if (has_values) {
                    ctx.symbols
                    |> SymbolTable.declare_value(
                         fst(id),
                         Valid(Module(module_values)),
                       );
                  };

                  let decorated_type =
                    Type.Valid(Module(module_types @ module_values));
                  valid_decorators
                  |> List.iter(((id, args)) =>
                       ctx.symbols
                       |> SymbolTable.declare_decorated(
                            id,
                            args
                            |> List.map(
                                 SymbolTable.Primitive.(
                                   Primitive.fold(
                                     ~nil=() => Nil,
                                     ~boolean=value => Boolean(value),
                                     ~integer=value => Integer(value),
                                     ~float=
                                       ((value, precision)) =>
                                         Float(value, precision),
                                     ~string=value => String(value),
                                   )
                                 ),
                               ),
                            decorated_type,
                          )
                     );

                  Node.raw(
                    (id, fst(statements), decorators' |> List.map(fst))
                    |> Interface.of_module,
                    Node.get_range(statements),
                  );
                }
              );
            }
          )
        )
    );

let parse_decorator: type_module_parser_t('ast) =
  ctx =>
    Parse.Util.define_statement(
      Matchers.keyword(Constants.Keyword.decorator),
      TypeExpression.parse
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      >>= (
        args =>
          Matchers.keyword(Constants.Keyword.on)
          >> __module_keyword
          >|= Node.get_range
          % (
            range => (
              (fst(args), Type.DecoratorTarget.Module),
              Some(range),
            )
          )
      )
      |> Matchers.terminated,
      ((id, (args, target))) => {
        let arg_types =
          args |> List.map(fst % TypeExpression.analyze(ctx.symbols));
        let type_ = Type.Valid(Decorator(arg_types, target));

        ctx.symbols |> SymbolTable.declare_value(fst(id), type_);

        (id, args, target) |> Interface.of_decorator;
      },
    );

let parse: type_module_parser_t('ast) =
  ctx => choice([parse_decorator(ctx), parse_module(ctx)]);
