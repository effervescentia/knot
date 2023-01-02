open Knot.Kore;
open Parse.Kore;
open AST;

type input_t = LazyStream.t(Input.t);
type output_t = option((Module.program_t, input_t));
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
  (Type.t => string, Module.module_statement_t) => Fmt.xml_t(string) =
  dump_type =>
    Dump.node_to_xml(
      ~unpack=
        Module.(
          fun
          | StdlibImport(named_imports) =>
            Fmt.Node(
              "StdlibImport",
              [],
              named_imports
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
          | Import(namespace, main_import, named_imports) =>
            (namespace, main_import, named_imports) |> KImport.Plugin.to_xml
          | Export(export, name, declaration) =>
            (export, name, declaration)
            |> KDeclaration.Plugin.to_xml(dump_type)
        )
        % (x => [x]),
      "ModuleStatement",
    );

let program_to_xml: (Type.t => string, Module.program_t) => Fmt.xml_t(string) =
  (dump_type, program) =>
    Node(
      "Program",
      [],
      program |> List.map(module_statement_to_xml(dump_type)),
    );
