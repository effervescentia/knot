open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;
module ParseContext = AST.ParseContext;
module SymbolTable = AST.SymbolTable;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module Type = AST.Type;
module Util = Parse.Util;

type type_module_parser_t =
  ParseContext.t => Parse.Parser.t(AST.TypeDefinition.module_t);

let _module_keyword = Matchers.keyword(Constants.Keyword.module_);

let _module_decorator = (ctx: ParseContext.t) =>
  KDecorator.Plugin.parse(KPrimitive.Plugin.parse)
  >|= Node.map(((id, args)) =>
        (
          id
          |> Node.add_type(
               ctx.symbols |> SymbolTable.resolve_value(fst(id)),
             ),
          args
          |> List.map(arg =>
               arg |> Node.add_type(arg |> fst |> KPrimitive.Plugin.analyze)
             ),
        )
      );

let module_: type_module_parser_t =
  ctx =>
    _module_decorator(ctx)
    |> many
    >>= (
      decorators =>
        _module_keyword
        >> (
          Matchers.identifier(~prefix=Matchers.alpha)
          >>= (
            id => {
              let module_ctx = ParseContext.create_module(ctx);

              KTypeStatement.Plugin.parse(module_ctx)
              |> many
              |> Matchers.between_braces
              >|= (
                stmts => {
                  let SymbolTable.Symbols.{types, values} =
                    module_ctx.symbols.declared;
                  let module_types =
                    types
                    |> List.map(
                         Tuple.map_snd2(type_ => Type.Container.Type(type_)),
                       );
                  let module_values =
                    values
                    |> List.map(
                         Tuple.map_snd2(type_ => Type.Container.Value(type_)),
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
                         Valid(`Module(module_types)),
                       );
                  };

                  if (has_values) {
                    ctx.symbols
                    |> SymbolTable.declare_value(
                         fst(id),
                         Valid(`Module(module_values)),
                       );
                  };

                  let decorated_type =
                    Type.Valid(`Module(module_types @ module_values));
                  valid_decorators
                  |> List.iter(((id, args)) =>
                       ctx.symbols
                       |> SymbolTable.declare_decorated(
                            id,
                            args,
                            decorated_type,
                          )
                     );

                  Node.untyped(
                    (id, fst(stmts), decorators' |> List.map(fst))
                    |> TD.of_module,
                    Node.get_range(stmts),
                  );
                }
              );
            }
          )
        )
    );

let decorator: type_module_parser_t =
  ctx =>
    Util.define_statement(
      Matchers.keyword(Constants.Keyword.decorator),
      KTypeExpression.Plugin.parse
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      >>= (
        args =>
          Matchers.keyword(Constants.Keyword.on)
          >> _module_keyword
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
          args |> List.map(fst % KTypeExpression.Plugin.analyze(ctx.symbols));
        let type_ = Type.Valid(`Decorator((arg_types, target)));

        ctx.symbols |> SymbolTable.declare_value(fst(id), type_);

        (id, args, target) |> TD.of_decorator;
      },
    );

let type_definition: type_module_parser_t =
  ctx => choice([decorator(ctx), module_(ctx)]);
