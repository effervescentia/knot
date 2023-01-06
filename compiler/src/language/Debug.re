open Kore;

let program_to_xml: AST.Module.program_t => Fmt.xml_t(string) =
  program =>
    Node("Program", [], program |> List.map(KModuleStatement.Plugin.to_xml));
