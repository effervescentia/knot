open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;
module ParseContext = AST.ParseContext;

type input_t = LazyStream.t(Input.t);
type output_t = option((AST.Module.program_t, input_t));
type t = (ParseContext.t, input_t) => output_t;

let _program = x => x << (eof() |> Matchers.lexeme);

let imports: t =
  ctx => choice([KImport.Plugin.parse(ctx), any >> none]) |> many;

let main: t =
  ctx =>
    choice([KImport.Plugin.parse(ctx), KDeclaration.Plugin.parse(ctx)])
    |> many
    |> _program;

let definition = (ctx: ParseContext.t) =>
  KTypeDefinition.Plugin.parse(ctx) |> many |> _program;

let module_statement_to_xml:
  (AST.Type.t => string, AST.Module.module_statement_t) => Fmt.xml_t(string) =
  dump_type =>
    Dump.node_to_xml(
      ~unpack=
        AST.Module.(
          fun
          | StandardImport(names) =>
            Fmt.Node(
              "StandardImport",
              [],
              names
              |> List.map(
                   Dump.node_to_xml(
                     ~unpack=
                       ((name, alias)) =>
                         [
                           Dump.node_to_xml(~dump_value=Fun.id, "Name", name),
                           ...alias
                              |> Option.map(alias' =>
                                   [
                                     Dump.node_to_xml(
                                       ~dump_value=Fun.id,
                                       "Alias",
                                       alias',
                                     ),
                                   ]
                                 )
                              |?: [],
                         ],
                     "Import",
                   ),
                 ),
            )
          | Import(namespace, imports) =>
            KImport.Plugin.to_xml((namespace, imports))
          | Declaration(name, decl) =>
            KDeclaration.Plugin.to_xml(dump_type, (name, decl))
        )
        % (x => [x]),
      "ModuleStatement",
    );

let program_to_xml:
  (AST.Type.t => string, AST.Module.program_t) => Fmt.xml_t(string) =
  (dump_type, program) =>
    Node(
      "Program",
      [],
      program |> List.map(module_statement_to_xml(dump_type)),
    );
