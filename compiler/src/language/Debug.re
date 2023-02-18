open Kore;

let program_to_xml:
  ('typ => string, Interface.program_t('typ)) => Fmt.xml_t(string) =
  (dump_type, program) =>
    Node(
      "Program",
      [],
      program
      |> List.map(
           KModuleStatement.Plugin.to_xml(
             KDeclaration.Plugin.to_xml(dump_type),
           ),
         ),
    );
